/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "mdns_stub.h"
#include "database/builder.h"

namespace opendeck::protocol::mdns
{
    /**
     * @brief Convenience builder that wires the disabled mDNS backend.
     */
    class Builder
    {
        public:
        explicit Builder([[maybe_unused]] database::Admin& database)
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
