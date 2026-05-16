/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/protocol/mdns/instance/stub/mdns_stub.h"
#include "firmware/src/database/builder/builder.h"
#include "firmware/src/mcu/shared/deps.h"

namespace opendeck::protocol::mdns
{
    /**
     * @brief Convenience builder that wires the disabled mDNS backend.
     */
    class Builder
    {
        public:
        /**
         * @brief Constructs the disabled mDNS builder.
         *
         * @param database Unused database administrator retained for builder compatibility.
         */
        explicit Builder([[maybe_unused]] database::Admin& database)
        {}

        /**
         * @brief Constructs the disabled mDNS builder with shared MCU services.
         *
         * @param database Unused database administrator retained for builder compatibility.
         * @param mcu Unused MCU services retained for builder compatibility.
         */
        Builder([[maybe_unused]] database::Admin& database,
                [[maybe_unused]] mcu::Hwa&        mcu)
        {}

        /**
         * @brief Returns the disabled mDNS backend.
         *
         * @return Stub mDNS backend.
         */
        Mdns& instance()
        {
            return _instance;
        }

        private:
        Mdns _instance;
    };
}    // namespace opendeck::protocol::mdns
