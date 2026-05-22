/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "bootloader/src/protocols/mdns/shared/deps.h"

namespace opendeck::bootloader::protocols::mdns
{
    /**
     * @brief Zephyr DNS-SD service descriptors advertised by bootloader mDNS.
     */
    class ServicesHw : public Services
    {
        public:
        /**
         * @brief Returns the mutable DFU DNS-SD service descriptor.
         *
         * @return DFU service descriptor.
         */
        opendeck::common::protocols::mdns::Service dfu() override;
    };
}    // namespace opendeck::bootloader::protocols::mdns
