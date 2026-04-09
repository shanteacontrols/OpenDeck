/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#if defined(CONFIG_PROJECT_TARGET_DRIVER_ANALOG_INPUT_NATIVE)
#include "native/count.h"
#elif defined(CONFIG_PROJECT_TARGET_DRIVER_ANALOG_INPUT_MULTIPLEXER)
#include "multiplexer/count.h"
#elif defined(CONFIG_PROJECT_TARGET_DRIVER_ANALOG_INPUT_MUXONMUX)
#include "mux_on_mux/count.h"
#else
#define OPENDECK_ANALOG_PHYSICAL_COUNT 0
#define OPENDECK_ANALOG_HAS_REMAP      0
#define OPENDECK_ANALOG_LOGICAL_COUNT  0
#endif
