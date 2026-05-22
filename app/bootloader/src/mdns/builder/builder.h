/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#ifdef CONFIG_PROJECT_BOOTLOADER_SUPPORT_MDNS
#include "bootloader/src/mdns/builder/hw/builder_hw.h"
#else
#include "bootloader/src/mdns/builder/stub/builder_stub.h"
#endif
