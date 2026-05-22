/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#ifdef OPENDECK_TEST
#include "bootloader/src/dfu/direct_update_writer/builder/test/builder_test.h"
#else
#include "bootloader/src/dfu/direct_update_writer/builder/hw/builder_hw.h"
#endif
