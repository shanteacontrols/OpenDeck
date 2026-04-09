/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#if defined(CONFIG_PROJECT_TARGET_DRIVER_DIGITAL_INPUT_NATIVE)
#include "native/count.h"
#elif defined(CONFIG_PROJECT_TARGET_DRIVER_DIGITAL_INPUT_SHIFT_REGISTER)
#include "shift_register/count.h"
#elif defined(CONFIG_PROJECT_TARGET_DRIVER_DIGITAL_INPUT_MATRIX_NATIVE_ROWS)
#include "matrix_native_rows/count.h"
#elif defined(CONFIG_PROJECT_TARGET_DRIVER_DIGITAL_INPUT_MATRIX_SHIFT_REGISTER_ROWS)
#include "matrix_shift_register_rows/count.h"
#else
#define OPENDECK_BUTTON_PHYSICAL_COUNT 0
#define OPENDECK_BUTTON_HAS_REMAP      0
#define OPENDECK_BUTTON_LOGICAL_COUNT  0
#define OPENDECK_ENCODER_COUNT         0
#define OPENDECK_DIGITAL_INPUT_COUNT   0
#endif
