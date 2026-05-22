/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "bootloader/src/mdns/services/hw/services_hw.h"

#ifdef CONFIG_PROJECT_BOOTLOADER_SUPPORT_MDNS

#include "common/src/mdns/shared/common.h"
#include "firmware/src/protocol/webconfig/shared/common.h"

#include <zephyr/net/dns_sd.h>
#include <zephyr/net/hostname.h>

#include <string_view>

using namespace opendeck::bootloader::mdns;

namespace
{
    constexpr std::string_view RECOVERY_SERVICE = "_opendeck-recovery";
    constexpr char             RECOVERY_TXT[]   = "\x0e"
                                                  "path=/recovery";

    char     recovery_instance[NET_HOSTNAME_SIZE] = CONFIG_NET_HOSTNAME;
    uint16_t recovery_port                        = 0;

    DNS_SD_REGISTER_SERVICE(opendeck_recovery,
                            recovery_instance,
                            RECOVERY_SERVICE.data(),
                            opendeck::mdns::TCP_PROTOCOL.data(),
                            opendeck::mdns::LOCAL_DOMAIN.data(),
                            RECOVERY_TXT,
                            &recovery_port);
}    // namespace

opendeck::mdns::Service ServicesHw::recovery()
{
    return {
        .instance     = recovery_instance,
        .port         = recovery_port,
        .service_port = protocol::webconfig::DEFAULT_PORT,
    };
}

#endif
