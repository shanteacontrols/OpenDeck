/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#ifdef OPENDECK_TEST
#include "bootloader/src/staged_update_reader/builder/test/builder_test.h"
#else
#include "bootloader/src/staged_update_reader/builder/hw/builder_hw.h"
#endif
