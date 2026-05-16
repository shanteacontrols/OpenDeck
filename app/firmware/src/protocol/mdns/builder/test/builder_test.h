/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/protocol/mdns/hwa/test/hwa_test.h"
#include "firmware/src/protocol/mdns/instance/impl/mdns.h"
#include "firmware/src/database/builder/builder.h"

namespace opendeck::protocol::mdns
{
    /**
     * @brief Convenience builder that wires the mDNS test backend.
     */
    class Builder
    {
        public:
        Builder()
            : _database(_default_database_builder.instance())
            , _instance(_hwa, _database)
        {}

        /**
         * @brief Constructs the mDNS test builder around the shared database instance.
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
        database::Builder _default_database_builder;
        HwaTest           _hwa;
        Database          _database;
        Mdns              _instance;
    };
}    // namespace opendeck::protocol::mdns
