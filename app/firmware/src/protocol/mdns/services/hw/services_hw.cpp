/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "firmware/src/protocol/mdns/services/hw/services_hw.h"

#ifdef CONFIG_PROJECT_TARGET_SUPPORT_MDNS

#include "firmware/src/protocol/mdns/shared/common.h"
#include "firmware/src/protocol/osc/shared/common.h"
#include "firmware/src/protocol/webconfig/shared/common.h"

#include <zephyr/net/dns_sd.h>
#include <zephyr/net/hostname.h>

using namespace opendeck::protocol::mdns;

namespace
{
    char     webconfig_instance[NET_HOSTNAME_SIZE] = CONFIG_NET_HOSTNAME;
    uint16_t webconfig_port                        = 0;
    char     osc_instance[NET_HOSTNAME_SIZE]       = CONFIG_NET_HOSTNAME;
    uint16_t osc_port                              = 0;

    DNS_SD_REGISTER_SERVICE(opendeck_webconfig,
                            webconfig_instance,
                            WEBCONFIG_SERVICE.data(),
                            opendeck::mdns::TCP_PROTOCOL.data(),
                            opendeck::mdns::LOCAL_DOMAIN.data(),
                            WEBCONFIG_TXT,
                            &webconfig_port);

    DNS_SD_REGISTER_SERVICE(opendeck_osc,
                            osc_instance,
                            OSC_SERVICE.data(),
                            opendeck::mdns::UDP_PROTOCOL.data(),
                            opendeck::mdns::LOCAL_DOMAIN.data(),
                            DNS_SD_EMPTY_TXT,
                            &osc_port);
}    // namespace

opendeck::mdns::Service ServicesHw::webconfig()
{
    return {
        .instance     = webconfig_instance,
        .port         = webconfig_port,
        .service_port = protocol::webconfig::DEFAULT_PORT,
    };
}

opendeck::mdns::Service ServicesHw::osc()
{
    return {
        .instance     = osc_instance,
        .port         = osc_port,
        .service_port = protocol::osc::DEFAULT_LISTEN_PORT,
    };
}

#endif
