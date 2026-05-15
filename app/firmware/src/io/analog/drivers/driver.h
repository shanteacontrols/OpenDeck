/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#if defined(CONFIG_PROJECT_TARGET_DRIVER_ANALOG_INPUT_NATIVE)
#include "firmware/src/io/analog/drivers/native/native_driver.h"
#elif defined(CONFIG_PROJECT_TARGET_DRIVER_ANALOG_INPUT_MULTIPLEXER)
#include "firmware/src/io/analog/drivers/multiplexer/multiplexer_driver.h"
#elif defined(CONFIG_PROJECT_TARGET_DRIVER_ANALOG_INPUT_MUXONMUX)
#include "firmware/src/io/analog/drivers/mux_on_mux/mux_on_mux_driver.h"
#else
#error "No analog driver selected through DT/Kconfig."
#endif
