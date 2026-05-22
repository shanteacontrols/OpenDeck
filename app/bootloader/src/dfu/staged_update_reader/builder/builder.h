/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#ifdef OPENDECK_TEST
#include "bootloader/src/dfu/staged_update_reader/builder/test/builder_test.h"
#elif defined(CONFIG_PROJECT_BOOTLOADER_STAGED_UPDATE)
#include "bootloader/src/dfu/staged_update_reader/builder/hw/builder_hw.h"
#else
#include "bootloader/src/dfu/staged_update_reader/builder/stub/builder_stub.h"
#endif
