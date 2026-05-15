/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#ifdef OPENDECK_TEST
#ifdef CONFIG_PROJECT_TARGET_SUPPORT_I2C
#include "firmware/src/io/i2c/builder_test.h"
#else
#include "firmware/src/io/i2c/builder_stub.h"
#endif
#else
#ifdef CONFIG_PROJECT_TARGET_SUPPORT_I2C
#include "firmware/src/io/i2c/builder_hw.h"
#else
#include "firmware/src/io/i2c/builder_stub.h"
#endif
#endif
