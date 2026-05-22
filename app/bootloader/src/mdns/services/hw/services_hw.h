/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "bootloader/src/mdns/shared/deps.h"

namespace opendeck::bootloader::mdns
{
    /**
     * @brief Zephyr DNS-SD service descriptors advertised by bootloader mDNS.
     */
    class ServicesHw : public Services
    {
        public:
        /**
         * @brief Returns the mutable recovery DNS-SD service descriptor.
         *
         * @return Recovery service descriptor.
         */
        opendeck::mdns::Service recovery() override;
    };
}    // namespace opendeck::bootloader::mdns
