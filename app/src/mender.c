// Copyright (c) 2026 ID8 Engineering AB
// SPDX-License-Identifier: Apache-2.0

#include <errno.h>
#include <stddef.h>

#include <modem/lte_lc.h>
#include <modem/modem_key_mgmt.h>
#include <modem/nrf_modem_lib.h>
#include <zephyr/storage/flash_map.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/reboot.h>

#include <mender/client.h>
#include <mender/zephyr-image-update-module.h>
#include <pm_config.h>

LOG_MODULE_REGISTER(mender_app, LOG_LEVEL_INF);

#ifndef CONFIG_MENDER_NET_CA_CERTIFICATE_TAG_PRIMARY
#define CONFIG_MENDER_NET_CA_CERTIFICATE_TAG_PRIMARY 1
#endif

#ifndef CONFIG_MENDER_NET_CA_CERTIFICATE_TAG_SECONDARY
#define CONFIG_MENDER_NET_CA_CERTIFICATE_TAG_SECONDARY 2
#endif

static char mender_identity_value[] = "nrf9151-dk-mender";
static mender_identity_t mender_identity = {
	.name = "device_id",
	.value = mender_identity_value,
};
static const unsigned char mender_ca_certificate[] =
#include "isrg_root_x1.pem.inc"
	;
static const unsigned char mender_artifact_ca_certificate[] =
#include "gts_root_r4.pem.inc"
	;
static bool lte_connected;
static K_MUTEX_DEFINE(lte_lock);

static int provision_ca_certificate(uint32_t tag, const unsigned char *certificate,
				    size_t certificate_len, const char *label)
{
	int ret;
	bool exists;
	int mismatch;

	ret = modem_key_mgmt_exists(tag, MODEM_KEY_MGMT_CRED_TYPE_CA_CHAIN, &exists);
	if (ret) {
		LOG_ERR("Failed to check %s: %d", label, ret);
		return ret;
	}

	if (exists) {
		mismatch = modem_key_mgmt_cmp(tag, MODEM_KEY_MGMT_CRED_TYPE_CA_CHAIN, certificate,
					      certificate_len);
		if (mismatch == 0) {
			return 0;
		}
		if (mismatch < 0) {
			LOG_ERR("Failed to compare %s: %d", label, mismatch);
			return mismatch;
		}

		ret = modem_key_mgmt_delete(tag, MODEM_KEY_MGMT_CRED_TYPE_CA_CHAIN);
		if (ret) {
			LOG_ERR("Failed to delete old %s: %d", label, ret);
			return ret;
		}
	}

	ret = modem_key_mgmt_write(tag, MODEM_KEY_MGMT_CRED_TYPE_CA_CHAIN, certificate,
				   certificate_len);
	if (ret) {
		LOG_ERR("Failed to write %s: %d", label, ret);
		return ret;
	}

	return 0;
}

static int provision_mender_ca_certificates(void)
{
	int ret;

	ret = provision_ca_certificate(CONFIG_MENDER_NET_CA_CERTIFICATE_TAG_PRIMARY,
				       mender_ca_certificate, sizeof(mender_ca_certificate),
				       "Mender API CA certificate");
	if (ret) {
		return ret;
	}

	ret = provision_ca_certificate(
		CONFIG_MENDER_NET_CA_CERTIFICATE_TAG_SECONDARY, mender_artifact_ca_certificate,
		sizeof(mender_artifact_ca_certificate), "Mender artifact CA certificate");
	if (ret) {
		return ret;
	}

	return 0;
}

static mender_err_t mender_network_connect_cb(void)
{
	int err;

	k_mutex_lock(&lte_lock, K_FOREVER);

	if (lte_connected) {
		k_mutex_unlock(&lte_lock);
		return MENDER_OK;
	}

	LOG_INF("Connecting to LTE network");
	err = lte_lc_connect();
	if (err) {
		LOG_ERR("LTE attach failed: %d", err);
		k_mutex_unlock(&lte_lock);
		return MENDER_FAIL;
	}

	lte_connected = true;
	LOG_INF("LTE network connected");
	k_mutex_unlock(&lte_lock);

	return MENDER_OK;
}

static mender_err_t mender_network_release_cb(void)
{
	/* Keep LTE attached between requests to avoid reconnect churn. */
	return MENDER_OK;
}

static mender_err_t mender_restart_cb(void)
{
	LOG_INF("Mender requested restart");
	sys_reboot(SYS_REBOOT_COLD);
	return MENDER_OK;
}

static mender_err_t mender_deployment_status_cb(mender_deployment_status_t status,
						const char *status_name)
{
	ARG_UNUSED(status);
	LOG_INF("Mender deployment status: %s", status_name);
	return MENDER_OK;
}

static mender_err_t mender_get_identity_cb(const mender_identity_t **identity)
{
	if (identity == NULL) {
		return MENDER_FAIL;
	}

	*identity = &mender_identity;
	return MENDER_OK;
}

mender_err_t mender_get_storage_spec(const struct device **dev, int *part_id,
				     uint16_t *sector_offset)
{
	const struct flash_area *fa;
	int ret;

	if ((dev == NULL) || (part_id == NULL) || (sector_offset == NULL)) {
		return MENDER_FAIL;
	}

	ret = flash_area_open(PM_NVS_STORAGE_ID, &fa);
	if (ret != 0) {
		LOG_ERR("Unable to open NVS storage flash area: %d", ret);
		return MENDER_FAIL;
	}

	*dev = flash_area_get_device(fa);
	*part_id = PM_NVS_STORAGE_ID;
	*sector_offset = 0;
	flash_area_close(fa);

	return MENDER_OK;
}

int mender_start(void)
{
	int modem_ret;
	mender_err_t ret;
	mender_client_config_t config = {
		.device_type = CONFIG_MENDER_DEVICE_TYPE,
		.host = CONFIG_MENDER_SERVER_HOST,
		.tenant_token = CONFIG_MENDER_SERVER_TENANT_TOKEN,
		.device_tier = NULL,
		.update_poll_interval = 0,
#ifndef CONFIG_MENDER_CLIENT_INVENTORY_DISABLE
		.inventory_update_interval = 0,
#endif
		.backoff_interval = 0,
		.max_backoff_interval = 0,
		.recommissioning = false,
	};
	mender_client_callbacks_t callbacks = {
		.network_connect = mender_network_connect_cb,
		.network_release = mender_network_release_cb,
		.deployment_status = mender_deployment_status_cb,
		.restart = mender_restart_cb,
		.get_identity = mender_get_identity_cb,
		.get_user_provided_keys = NULL,
	};

	modem_ret = nrf_modem_lib_init();
	if (modem_ret) {
		LOG_ERR("Modem library initialization failed: %d", modem_ret);
		return -EINVAL;
	}

	modem_ret = provision_mender_ca_certificates();
	if (modem_ret) {
		return -EINVAL;
	}

	ret = mender_client_init(&config, &callbacks);
	if (ret != MENDER_OK) {
		LOG_ERR("Mender init failed: %d", ret);
		return -EINVAL;
	}

	ret = mender_zephyr_image_register_update_module();
	if (ret != MENDER_OK) {
		LOG_ERR("Mender update module registration failed: %d", ret);
		return -EINVAL;
	}

	ret = mender_client_activate();
	if (ret != MENDER_OK) {
		LOG_ERR("Mender activation failed: %d", ret);
		return -EINVAL;
	}

	LOG_INF("Mender activated");
	return 0;
}
