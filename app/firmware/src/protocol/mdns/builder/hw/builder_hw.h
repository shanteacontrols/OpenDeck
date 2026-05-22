/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/protocol/mdns/hwa/hw/hwa_hw.h"
#include "firmware/src/protocol/mdns/instance/impl/mdns.h"
#include "firmware/src/protocol/mdns/services/hw/services_hw.h"
#include "firmware/src/database/builder/builder.h"
#include "common/src/mdns/instance/impl/mdns.h"

namespace opendeck::protocol::mdns
{
    /**
     * @brief Convenience builder that wires the mDNS discovery backend.
     */
    class Builder
    {
        public:
        /**
         * @brief Constructs the mDNS builder around shared database and MCU services.
         *
         * @param database Database administrator used for device-wide settings.
         * @param mcu MCU services shared by firmware subsystems.
         */
        Builder(database::Admin& database, mcu::Hwa& mcu)
            : _database(database)
            , _hwa(mcu)
            , _base_mdns(_hwa)
            , _instance(_base_mdns, _services, _database)
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
        Database                 _database;
        HwaHw                    _hwa;
        opendeck::mdns::BaseMdns _base_mdns;
        ServicesHw               _services;
        Mdns                     _instance;
    };
}    // namespace opendeck::protocol::mdns
