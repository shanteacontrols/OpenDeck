/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#ifdef OPENDECK_TEST
#ifdef CONFIG_PROJECT_TARGET_SUPPORT_ADC
#include "firmware/src/io/analog/builder/test/builder_test.h"
#else
#include "firmware/src/io/analog/builder/stub/builder_stub.h"
#endif
#else
#ifdef CONFIG_PROJECT_TARGET_SUPPORT_ADC
#include "firmware/src/io/analog/builder/hw/builder_hw.h"
#else
#include "firmware/src/io/analog/builder/stub/builder_stub.h"
#endif
#endif
