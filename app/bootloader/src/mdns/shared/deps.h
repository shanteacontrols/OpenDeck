/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "common/src/mdns/shared/common.h"

namespace opendeck::bootloader::mdns
{
    /**
     * @brief DNS-SD services advertised by the bootloader mDNS backend.
     */
    class Services
    {
        public:
        virtual ~Services() = default;

        /**
         * @brief Returns the mutable recovery DNS-SD service descriptor.
         *
         * @return Recovery service descriptor.
         */
        virtual opendeck::mdns::Service recovery() = 0;
    };
}    // namespace opendeck::bootloader::mdns
