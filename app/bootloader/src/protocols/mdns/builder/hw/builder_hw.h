/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "bootloader/src/protocols/mdns/instance/impl/mdns.h"
#include "bootloader/src/protocols/mdns/services/hw/services_hw.h"
#include "common/src/mcu/shared/deps.h"
#include "common/src/protocols/mdns/hwa/hw/hwa_hw.h"

namespace opendeck::bootloader::protocols::mdns
{
    /**
     * @brief Convenience builder that wires bootloader mDNS discovery.
     */
    class Builder
    {
        public:
        explicit Builder(opendeck::common::mcu::Hwa& mcu)
            : _hwa(mcu)
            , _base_mdns(_hwa)
            , _instance(_base_mdns, _services)
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
        opendeck::common::protocols::mdns::HwaHw    _hwa;
        opendeck::common::protocols::mdns::BaseMdns _base_mdns;
        ServicesHw                                  _services;
        Mdns                                        _instance;
    };
}    // namespace opendeck::bootloader::protocols::mdns
