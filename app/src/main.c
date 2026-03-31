// Copyright (c) 2026 ID8 Engineering AB
// SPDX-License-Identifier: Apache-2.0

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "mender.h"

LOG_MODULE_REGISTER(app, LOG_LEVEL_INF);

int main(void)
{
	int ret = mender_start();
	if (ret < 0) {
		LOG_ERR("Mender start failed: %d", ret);
		return ret;
	}

	LOG_INF("VERSION v0.1");

	while (1) {
		k_sleep(K_SECONDS(1));
	}
}
