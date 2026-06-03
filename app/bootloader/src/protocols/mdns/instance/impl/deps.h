/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "common/src/protocols/mdns/shared/common.h"

namespace opendeck::bootloader::protocols::mdns
{
    /**
     * @brief DNS-SD services advertised by the bootloader mDNS backend.
     */
    class Services
    {
        public:
        virtual ~Services() = default;

        /**
         * @brief Returns the mutable DFU DNS-SD service descriptor.
         *
         * @return DFU service descriptor.
         */
        virtual opendeck::common::protocols::mdns::Service dfu() = 0;
    };
}    // namespace opendeck::bootloader::protocols::mdns
