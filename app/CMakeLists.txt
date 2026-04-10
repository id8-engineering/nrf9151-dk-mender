# Copyright (c) 2026 ID8 Engineering AB
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.13.1)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(app LANGUAGES C)

set(MENDER_CLIENT_VERSION "local")

target_sources(app PRIVATE
  src/main.c
  ${ZEPHYR_BASE}/modules/mbedtls/zephyr_entropy.c
  src/mender_net_zephyr/http.c
  src/mender_net_zephyr/net.c
)

set_source_files_properties(
  ${ZEPHYR_BASE}/modules/mbedtls/zephyr_entropy.c
  PROPERTIES
  COMPILE_DEFINITIONS CONFIG_MBEDTLS_ENTROPY_POLL_ZEPHYR=1
)

target_include_directories(app PRIVATE
  ${ZEPHYR_BASE}/../modules/mender-mcu/include
  ${ZEPHYR_BASE}/../modules/mender-mcu/src/include
  ${ZEPHYR_BASE}/../modules/mender-mcu/src/platform/net/zephyr
)

target_compile_definitions(app PRIVATE
  MENDER_CLIENT_VERSION=\"${MENDER_CLIENT_VERSION}\"
)
