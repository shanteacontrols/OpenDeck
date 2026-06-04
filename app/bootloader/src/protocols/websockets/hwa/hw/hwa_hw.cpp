/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "bootloader/src/protocols/websockets/hwa/hw/hwa_hw.h"

#include "common/src/protocols/websockets/shared/common.h"

#include <zephyr/net/http/service.h>
#include <zephyr/sys/util.h>

using namespace opendeck::bootloader::protocols::websockets;

namespace
{
    uint16_t http_port = opendeck::common::protocols::websockets::DEFAULT_PORT;
    uint8_t  upgrade_buffer[opendeck::common::protocols::websockets::UPGRADE_BUFFER_SIZE];

    http_resource_detail_websocket websockets_resource_detail = {
        .common = {
            .bitmask_of_supported_http_methods = BIT(HTTP_GET),
            .type                              = HTTP_RESOURCE_TYPE_WEBSOCKET,
        },
        .ws_sock         = -1,
        .cb              = HwaHw::setup,
        .data_buffer     = upgrade_buffer,
        .data_buffer_len = sizeof(upgrade_buffer),
        .user_data       = nullptr,
    };
}    // namespace

// NOLINTNEXTLINE(misc-use-anonymous-namespace): Zephyr macro defines service storage with internal linkage.
HTTP_SERVICE_DEFINE(bootloader_websockets_service,
                    nullptr,
                    &http_port,
                    opendeck::common::protocols::websockets::CLIENT_COUNT,
                    1,
                    nullptr,
                    nullptr,
                    nullptr);
HTTP_RESOURCE_DEFINE(bootloader_websockets_resource, bootloader_websockets_service, "/dfu", &websockets_resource_detail);

HwaHw::HwaHw()
    : opendeck::common::protocols::websockets::HwaHw(websockets_resource_detail)
{}
