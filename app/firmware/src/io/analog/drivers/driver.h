/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <zephyr/drivers/adc.h>

#define OPENDECK_ANALOG_ADC_CHANNEL_NODE                                                    \
    DT_CHILD_BY_UNIT_ADDR_INT(DT_IO_CHANNELS_CTLR_BY_IDX(DT_NODELABEL(opendeck_analog), 0), \
                              DT_IO_CHANNELS_INPUT_BY_IDX(DT_NODELABEL(opendeck_analog), 0))

#define OPENDECK_ANALOG_ADC_BITS DT_PROP_OR(OPENDECK_ANALOG_ADC_CHANNEL_NODE, zephyr_resolution, 0)

#if defined(CONFIG_PROJECT_TARGET_DRIVER_ANALOG_INPUT_NATIVE)
#include "native/native_driver.h"
#elif defined(CONFIG_PROJECT_TARGET_DRIVER_ANALOG_INPUT_MULTIPLEXER)
#include "multiplexer/multiplexer_driver.h"
#elif defined(CONFIG_PROJECT_TARGET_DRIVER_ANALOG_INPUT_MUXONMUX)
#include "mux_on_mux/mux_on_mux_driver.h"
#else
#error "No analog driver selected through DT/Kconfig."
#endif
