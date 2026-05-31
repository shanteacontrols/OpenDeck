/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "firmware/src/protocol/mdns/services/hw/services_osc.h"

#include "firmware/src/protocol/mdns/shared/common.h"
#include "firmware/src/protocol/osc/shared/common.h"

#include <zephyr/net/dns_sd.h>
#include <zephyr/net/hostname.h>

using namespace opendeck::protocol::mdns;

namespace
{
    char     osc_instance[NET_HOSTNAME_SIZE] = CONFIG_NET_HOSTNAME;
    uint16_t osc_port                        = 0;

    DNS_SD_REGISTER_SERVICE(opendeck_osc,
                            osc_instance,
                            opendeck::protocol::mdns::OSC_SERVICE.data(),
                            opendeck::common::protocols::mdns::UDP_PROTOCOL.data(),
                            opendeck::common::protocols::mdns::LOCAL_DOMAIN.data(),
                            DNS_SD_EMPTY_TXT,
                            &osc_port);
}    // namespace

OscService::OscService()
{
    ServicesHw::register_service(this);
}

opendeck::common::protocols::mdns::Service OscService::service()
{
    return {
        .instance     = osc_instance,
        .port         = osc_port,
        .service_port = opendeck::protocol::osc::DEFAULT_LISTEN_PORT,
    };
}
