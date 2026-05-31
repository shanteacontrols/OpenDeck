/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/protocol/mdns/instance/impl/mdns.h"
#include "firmware/src/protocol/mdns/services/hw/services_hw.h"
#ifdef CONFIG_PROJECT_TARGET_SUPPORT_OSC
#include "firmware/src/protocol/mdns/services/hw/services_osc.h"
#endif
#ifdef CONFIG_PROJECT_TARGET_SUPPORT_WEBSOCKETS
#include "firmware/src/protocol/mdns/services/hw/services_websockets.h"
#endif
#include "firmware/src/database/builder/builder.h"
#include "common/src/protocols/mdns/hwa/hw/hwa_hw.h"
#include "common/src/protocols/mdns/instance/impl/mdns.h"

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
        Builder(database::Admin& database, opendeck::common::mcu::Hwa& mcu)
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
        Database                                    _database;
        opendeck::common::protocols::mdns::HwaHw    _hwa;
        opendeck::common::protocols::mdns::BaseMdns _base_mdns;
        ServicesHw                                  _services;
#ifdef CONFIG_PROJECT_TARGET_SUPPORT_OSC
        OscService _osc_service;
#endif
#ifdef CONFIG_PROJECT_TARGET_SUPPORT_WEBSOCKETS
        WebSocketsService _websockets_service;
#endif
        Mdns _instance;
    };
}    // namespace opendeck::protocol::mdns
