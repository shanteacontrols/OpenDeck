/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "bootloader/src/protocols/mdns/instance/stub/mdns_stub.h"
#include "common/src/mcu/shared/deps.h"

namespace opendeck::bootloader::protocols::mdns
{
    /**
     * @brief Convenience builder that wires stub bootloader mDNS discovery.
     */
    class Builder
    {
        public:
        /**
         * @brief Constructs the stub mDNS builder.
         *
         * @param mcu MCU dependency kept for the shared builder shape.
         */
        explicit Builder([[maybe_unused]] opendeck::common::mcu::Hwa& mcu)
        {}

        /**
         * @brief Returns the stub mDNS discovery backend.
         *
         * @return Stub mDNS discovery backend.
         */
        MdnsStub& instance()
        {
            return _instance;
        }

        private:
        MdnsStub _instance;
    };
}    // namespace opendeck::bootloader::protocols::mdns
