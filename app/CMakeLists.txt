# Copyright (c) 2026 ID8 Engineering AB
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.13.1)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(app LANGUAGES C)

target_sources(app PRIVATE
  src/main.c
  ${ZEPHYR_BASE}/modules/mbedtls/zephyr_entropy.c # Mender client needs legacy crypto mbedTLS
)

# Must be compiled with the right CONFIG option to enable mbedtls_hardware_poll()
set_source_files_properties(
  ${ZEPHYR_BASE}/modules/mbedtls/zephyr_entropy.c
  PROPERTIES
  COMPILE_DEFINITIONS CONFIG_MBEDTLS_ENTROPY_POLL_ZEPHYR=1
)

