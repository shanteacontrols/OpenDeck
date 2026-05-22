/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#ifdef CONFIG_PROJECT_TARGET_SUPPORT_WEBSOCKETS

#include "firmware/src/protocol/websockets/hwa/hw/hwa_hw.h"
#include "firmware/src/protocol/websockets/instance/impl/websockets.h"
#include "common/src/websockets/shared/common.h"

#include <zephyr/net/http/service.h>
#include <zephyr/sys/util.h>

using namespace opendeck::protocol::websockets;

namespace
{
    uint16_t http_port = opendeck::websockets::DEFAULT_PORT;
    uint8_t  upgrade_buffer[opendeck::websockets::UPGRADE_BUFFER_SIZE];

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
HTTP_SERVICE_DEFINE(websockets_service, nullptr, &http_port, opendeck::websockets::CLIENT_COUNT, 1, nullptr, nullptr, nullptr);
HTTP_RESOURCE_DEFINE(websockets_resource, websockets_service, "/config", &websockets_resource_detail);

HwaHw::HwaHw()
    : opendeck::websockets::HwaHw(websockets_resource_detail)
{}

#endif
