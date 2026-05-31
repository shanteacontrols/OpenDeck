/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/protocol/mdns/shared/deps.h"
#include "firmware/src/protocol/osc/shared/common.h"
#include "common/src/protocols/websockets/shared/common.h"

#include <array>
#include <vector>

namespace opendeck::protocol::mdns
{
    /**
     * @brief In-memory DNS-SD service descriptors used by tests.
     */
    class ServicesTest : public Services
    {
        class Provider : public ServiceProvider
        {
            public:
            explicit Provider(opendeck::common::protocols::mdns::Service service)
                : _service(service)
            {}

            opendeck::common::protocols::mdns::Service service() override
            {
                return _service;
            }

            private:
            opendeck::common::protocols::mdns::Service _service;
        };

        public:
        std::array<char, opendeck::common::protocols::mdns::NETWORK_NAME_SIZE> websockets_instance = {};
        std::array<char, opendeck::common::protocols::mdns::NETWORK_NAME_SIZE> osc_instance        = {};
        uint16_t                                                               websockets_port     = 0;
        uint16_t                                                               osc_port            = 0;

        ServicesTest()
            : _websockets({
                  .instance     = websockets_instance,
                  .port         = websockets_port,
                  .service_port = opendeck::common::protocols::websockets::DEFAULT_PORT,
              })
            , _osc({
                  .instance     = osc_instance,
                  .port         = osc_port,
                  .service_port = protocol::osc::DEFAULT_LISTEN_PORT,
              })
            , _services({ &_websockets, &_osc })
        {}

        std::span<ServiceProvider* const> services() override
        {
            return std::span<ServiceProvider* const>(_services.data(), _services.size());
        }

        private:
        Provider                      _websockets;
        Provider                      _osc;
        std::vector<ServiceProvider*> _services;
    };
}    // namespace opendeck::protocol::mdns
