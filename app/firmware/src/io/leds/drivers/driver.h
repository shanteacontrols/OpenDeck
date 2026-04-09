/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#if defined(CONFIG_PROJECT_TARGET_DRIVER_DIGITAL_OUTPUT_NATIVE)
#include "native/native_driver.h"
#elif defined(CONFIG_PROJECT_TARGET_DRIVER_DIGITAL_OUTPUT_SHIFT_REGISTER)
#include "shift_register/shift_register_driver.h"
#elif defined(CONFIG_PROJECT_TARGET_DRIVER_DIGITAL_OUTPUT_MATRIX_NATIVE_ROWS)
#include "matrix_native_rows/matrix_native_rows_driver.h"
#elif defined(CONFIG_PROJECT_TARGET_DRIVER_DIGITAL_OUTPUT_MAX7219)
#include "max7219/max7219_driver.h"
#elif defined(CONFIG_PROJECT_TARGET_SUPPORT_TOUCHSCREEN) && \
    !defined(CONFIG_PROJECT_TARGET_SUPPORT_DIGITAL_OUTPUTS)
#include "stub/stub_driver.h"
#else
#error "No LED driver selected through DT/Kconfig."
#endif
