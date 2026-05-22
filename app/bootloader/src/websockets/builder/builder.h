/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#ifdef CONFIG_PROJECT_BOOTLOADER_SUPPORT_WEBSOCKETS
#include "bootloader/src/websockets/builder/hw/builder_hw.h"
#else
#include "bootloader/src/websockets/builder/stub/builder_stub.h"
#endif
