/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "common/src/mdns/hwa/hw/hwa_hw.h"
#include "common/src/mcu/shared/deps.h"

namespace opendeck::bootloader::mdns
{
    /**
     * @brief Bootloader mDNS hardware hooks.
     */
    class HwaHw : public opendeck::mdns::HwaHw
    {
        public:
        explicit HwaHw(mcu::Hwa& mcu);
    };
}    // namespace opendeck::bootloader::mdns
