/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "bootloader/src/mdns/hwa/hw/hwa_hw.h"
#include "bootloader/src/mdns/instance/impl/mdns.h"
#include "bootloader/src/mdns/services/hw/services_hw.h"
#include "common/src/mcu/shared/deps.h"

namespace opendeck::bootloader::mdns
{
    /**
     * @brief Convenience builder that wires bootloader mDNS discovery.
     */
    class Builder
    {
        public:
        explicit Builder(mcu::Hwa& mcu)
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
        HwaHw                    _hwa;
        opendeck::mdns::BaseMdns _base_mdns;
        ServicesHw               _services;
        Mdns                     _instance;
    };
}    // namespace opendeck::bootloader::mdns
