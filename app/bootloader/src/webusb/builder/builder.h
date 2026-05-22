/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#ifdef CONFIG_PROJECT_BOOTLOADER_SUPPORT_USB_DFU
#include "bootloader/src/webusb/builder/hw/builder_hw.h"
#else
#include "bootloader/src/webusb/builder/stub/builder_stub.h"
#endif
