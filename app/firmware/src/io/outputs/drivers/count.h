/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#if defined(CONFIG_PROJECT_TARGET_DRIVER_DIGITAL_OUTPUT_NATIVE)
#include "native/count.h"
#elif defined(CONFIG_PROJECT_TARGET_DRIVER_DIGITAL_OUTPUT_SHIFT_REGISTER)
#include "shift_register/count.h"
#elif defined(CONFIG_PROJECT_TARGET_DRIVER_DIGITAL_OUTPUT_MATRIX_NATIVE_ROWS)
#include "matrix_native_rows/count.h"
#elif defined(CONFIG_PROJECT_TARGET_DRIVER_DIGITAL_OUTPUT_MAX7219)
#include "max7219/count.h"
#else
#define OPENDECK_OUTPUT_PHYSICAL_COUNT 0
#define OPENDECK_OUTPUT_HAS_REMAP      0
#define OPENDECK_OUTPUT_LOGICAL_COUNT  0
#endif
