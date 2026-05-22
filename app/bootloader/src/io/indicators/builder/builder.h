/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#ifdef OPENDECK_TEST
#include "bootloader/src/io/indicators/builder/test/builder_test.h"
#elif defined(CONFIG_PROJECT_BOOTLOADER_SUPPORT_TRAFFIC_INDICATORS)
#include "bootloader/src/io/indicators/builder/hw/builder_hw.h"
#else
#include "bootloader/src/io/indicators/builder/stub/builder_stub.h"
#endif
