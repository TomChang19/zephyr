# Copyright (c) 2025 Ishraq Ibne Ashraf <ishraq.i.ashraf@gmail.com>
#
# SPDX-License-Identifier: Apache-2.0

zephyr_include_directories(.)

zephyr_sources(soc.c)

set(SOC_LINKER_SCRIPT ${ZEPHYR_BASE}/include/zephyr/arch/arm/cortex_m/scripts/linker.ld CACHE INTERNAL "")
