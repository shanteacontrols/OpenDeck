/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "bootloader/src/mdns/hwa/hw/hwa_hw.h"

#ifdef CONFIG_PROJECT_BOOTLOADER_SUPPORT_MDNS

using namespace opendeck::bootloader::mdns;

HwaHw::HwaHw(mcu::Hwa& mcu)
    : opendeck::mdns::HwaHw(mcu)
{}

#endif
