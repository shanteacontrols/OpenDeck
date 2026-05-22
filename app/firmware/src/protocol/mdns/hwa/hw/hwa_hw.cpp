/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "firmware/src/protocol/mdns/hwa/hw/hwa_hw.h"

#ifdef CONFIG_PROJECT_TARGET_SUPPORT_MDNS

using namespace opendeck::protocol::mdns;

HwaHw::HwaHw(mcu::Hwa& mcu)
    : opendeck::mdns::HwaHw(mcu)
{}

#endif
