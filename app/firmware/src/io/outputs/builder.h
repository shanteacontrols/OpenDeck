/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#ifdef OPENDECK_TEST
#ifdef CONFIG_PROJECT_TARGET_SUPPORT_OUTPUTS
#include "firmware/src/io/outputs/builder/test/builder_test.h"
#else
#include "firmware/src/io/outputs/builder/stub/builder_stub.h"
#endif
#else
#ifdef CONFIG_PROJECT_TARGET_SUPPORT_OUTPUTS
#include "firmware/src/io/outputs/builder/hw/builder_hw.h"
#else
#include "firmware/src/io/outputs/builder/stub/builder_stub.h"
#endif
#endif
