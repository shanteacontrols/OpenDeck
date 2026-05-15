/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#if defined(CONFIG_PROJECT_TARGET_DRIVER_DIGITAL_OUTPUT_NATIVE)
#include "firmware/src/io/outputs/drivers/native/native_driver.h"
#elif defined(CONFIG_PROJECT_TARGET_DRIVER_DIGITAL_OUTPUT_SHIFT_REGISTER)
#include "firmware/src/io/outputs/drivers/shift_register/shift_register_driver.h"
#elif defined(CONFIG_PROJECT_TARGET_DRIVER_DIGITAL_OUTPUT_MATRIX_NATIVE_ROWS)
#include "firmware/src/io/outputs/drivers/matrix_native_rows/matrix_native_rows_driver.h"
#elif defined(CONFIG_PROJECT_TARGET_DRIVER_DIGITAL_OUTPUT_MAX7219)
#include "firmware/src/io/outputs/drivers/max7219/max7219_driver.h"
#elif defined(CONFIG_PROJECT_TARGET_SUPPORT_TOUCHSCREEN) && \
    !defined(CONFIG_PROJECT_TARGET_SUPPORT_DIGITAL_OUTPUTS)
#include "firmware/src/io/outputs/drivers/stub/stub_driver.h"
#else
#error "No output driver selected through DT/Kconfig."
#endif
