// Copyright (c) 2026 ID8 Engineering AB
// SPDX-License-Identifier: Apache-2.0

#include <errno.h>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/reboot.h>
#include <zephyr/dfu/mcuboot.h>

#include <modem/lte_lc.h>
#include <modem/nrf_modem_lib.h>

#include <mender/client.h>
#include <mender/zephyr-image-update-module.h>

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

static mender_identity_t mender_identity = {
	.name = "device_id",
	.value = "nrf9151dk",
};

static mender_err_t mender_network_connect_cb(void)
{
	return MENDER_OK;
}

static mender_err_t mender_network_release_cb(void)
{
	return MENDER_OK;
}

static mender_err_t mender_deployment_status_cb(mender_deployment_status_t status, const char *desc)
{
	ARG_UNUSED(status);
	ARG_UNUSED(desc);
	return MENDER_OK;
}

static mender_err_t mender_restart_cb(void)
{
	LOG_WRN("Mender requested reboot; waiting 5s for logs");
	k_sleep(K_SECONDS(5));
	sys_reboot(SYS_REBOOT_COLD);
	return MENDER_OK;
}

static mender_err_t mender_get_identity_cb(const mender_identity_t **identity)
{
	*identity = &mender_identity;
	return MENDER_OK;
}

static void confirm_running_image(void)
{
	int err = boot_is_img_confirmed();

	if (err < 0) {
		LOG_WRN("Unable to read image confirmation state: %d", err);
		return;
	}

	if (err == 0) {
		err = boot_write_img_confirmed();
		if (err < 0) {
			LOG_ERR("Failed to confirm running image: %d", err);
		} else {
			LOG_INF("Running image confirmed");
		}
	}
}

int main(void)
{
	int err;

	LOG_INF("Starting LTE");

	err = nrf_modem_lib_init();
	if (err < 0) {
		LOG_ERR("Modem init failed: %d", err);
		return -EIO;
	}

	err = lte_lc_connect();
	if (err < 0) {
		LOG_ERR("LTE connect failed: %d", err);
		return -EIO;
	}

	LOG_INF("LTE connected");

	mender_client_config_t mender_config = {
		.recommissioning = false,
	};

	mender_client_callbacks_t mender_callbacks = {
		.network_connect = mender_network_connect_cb,
		.network_release = mender_network_release_cb,
		.deployment_status = mender_deployment_status_cb,
		.restart = mender_restart_cb,
		.get_identity = mender_get_identity_cb,
	};

	if (mender_client_init(&mender_config, &mender_callbacks) != MENDER_OK) {
		LOG_ERR("Mender init failed");
		return -EIO;
	}

#ifdef CONFIG_MENDER_ZEPHYR_IMAGE_UPDATE_MODULE
	if (mender_zephyr_image_register_update_module() != MENDER_OK) {
		LOG_ERR("Mender update module registration failed");
		return -EIO;
	}
#endif

	if (mender_client_activate() != MENDER_OK) {
		LOG_ERR("Mender activate failed");
		return -EIO;
	}

	LOG_INF("Mender activated");

	LOG_INF(" V0.1");

	while (1) {
		k_sleep(K_SECONDS(10));
	}

	return 0;
}
