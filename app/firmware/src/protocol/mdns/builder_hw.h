/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "hwa_hw.h"
#include "mdns.h"
#include "database/builder.h"

namespace opendeck::protocol::mdns
{
    /**
     * @brief Convenience builder that wires the mDNS discovery backend.
     */
    class Builder
    {
        public:
        /**
         * @brief Constructs the mDNS builder around the shared database instance.
         *
         * @param database Database administrator used for device-wide settings.
         */
        explicit Builder(database::Admin& database)
            : _database(database)
            , _instance(_hwa, _database)
        {}

        /**
         * @brief Returns the configured mDNS discovery backend.
         *
         * @return mDNS discovery backend.
         */
        Mdns& instance()
        {
            return _instance;
        }

        private:
        HwaHw    _hwa;
        Database _database;
        Mdns     _instance;
    };
}    // namespace opendeck::protocol::mdns
