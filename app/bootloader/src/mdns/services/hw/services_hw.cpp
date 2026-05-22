/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "bootloader/src/mdns/services/hw/services_hw.h"

#ifdef CONFIG_PROJECT_BOOTLOADER_SUPPORT_MDNS

#include "bootloader/src/mdns/shared/common.h"
#include "common/src/mdns/shared/common.h"
#include "common/src/websockets/shared/common.h"

#include <zephyr/net/dns_sd.h>
#include <zephyr/net/hostname.h>

using namespace opendeck::bootloader::mdns;

namespace
{
    char     dfu_instance[NET_HOSTNAME_SIZE] = CONFIG_NET_HOSTNAME;
    uint16_t dfu_port                        = 0;

    DNS_SD_REGISTER_SERVICE(opendeck_dfu,
                            dfu_instance,
                            DFU_SERVICE.data(),
                            opendeck::mdns::TCP_PROTOCOL.data(),
                            opendeck::mdns::LOCAL_DOMAIN.data(),
                            DFU_TXT,
                            &dfu_port);
}    // namespace

opendeck::mdns::Service ServicesHw::dfu()
{
    return {
        .instance     = dfu_instance,
        .port         = dfu_port,
        .service_port = opendeck::websockets::DEFAULT_PORT,
    };
}

#endif
