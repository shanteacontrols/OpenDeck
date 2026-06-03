/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "firmware/src/protocol/mdns/services/hw/services_websockets.h"

#include "firmware/src/protocol/mdns/services/hw/common.h"
#include "common/src/protocols/websockets/shared/common.h"

#include <zephyr/net/dns_sd.h>
#include <zephyr/net/hostname.h>

using namespace opendeck::firmware::protocol::mdns;

namespace
{
    char     websockets_instance[NET_HOSTNAME_SIZE] = CONFIG_NET_HOSTNAME;
    uint16_t websockets_port                        = 0;

    DNS_SD_REGISTER_SERVICE(opendeck_websockets,
                            websockets_instance,
                            opendeck::firmware::protocol::mdns::WEBSOCKETS_SERVICE.data(),
                            opendeck::common::protocols::mdns::TCP_PROTOCOL.data(),
                            opendeck::common::protocols::mdns::LOCAL_DOMAIN.data(),
                            opendeck::firmware::protocol::mdns::WEBSOCKETS_TXT,
                            &websockets_port);
}    // namespace

WebSocketsService::WebSocketsService()
{
    ServicesHw::register_service(this);
}

opendeck::common::protocols::mdns::Service WebSocketsService::service()
{
    return {
        .instance     = websockets_instance,
        .port         = websockets_port,
        .service_port = opendeck::common::protocols::websockets::DEFAULT_PORT,
    };
}
