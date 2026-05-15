/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#if defined(CONFIG_PROJECT_TARGET_SUPPORT_WEBCONFIG) && !defined(OPENDECK_TEST)

#include "firmware/src/protocol/webconfig/hwa_hw.h"
#include "firmware/src/protocol/webconfig/common.h"
#include "bootloader/src/fw_selector/common.h"
#include "firmware/src/protocol/webconfig/webconfig.h"

#include <zephyr/kernel.h>
#include <zephyr/net/http/server.h>
#include <zephyr/net/http/service.h>
#include <zephyr/net/websocket.h>
#include <zephyr/sys/util.h>

using namespace opendeck::protocol::webconfig;

namespace
{
    uint16_t http_port = DEFAULT_PORT;
    uint8_t  upgrade_buffer[UPGRADE_BUFFER_SIZE];

    /**
     * @brief Accepts a WebSocket upgrade and hands the socket to WebConfig.
     *
     * @param ws_socket Upgraded WebSocket socket descriptor.
     * @param request_ctx HTTP request context supplied by Zephyr.
     * @param user_data WebConfig instance attached to the resource.
     *
     * @return 0 on success, otherwise a negative errno value.
     */
    int webconfig_setup(int                                       ws_socket,
                        [[maybe_unused]] struct http_request_ctx* request_ctx,
                        void*                                     user_data)
    {
        auto self = static_cast<WebConfig*>(user_data);

        if (self == nullptr)
        {
            return -EINVAL;
        }

        return self->accept_client(ws_socket);
    }

    http_resource_detail_websocket webconfig_resource_detail = {
        .common = {
            .bitmask_of_supported_http_methods = BIT(HTTP_GET),
            .type                              = HTTP_RESOURCE_TYPE_WEBSOCKET,
        },
        .ws_sock         = -1,
        .cb              = webconfig_setup,
        .data_buffer     = upgrade_buffer,
        .data_buffer_len = sizeof(upgrade_buffer),
        .user_data       = nullptr,
    };
}    // namespace

// NOLINTNEXTLINE(misc-use-anonymous-namespace): Zephyr macro defines service storage with internal linkage.
HTTP_SERVICE_DEFINE(webconfig_service, nullptr, &http_port, CLIENT_COUNT, 1, nullptr, nullptr, nullptr);
HTTP_RESOURCE_DEFINE(webconfig_resource, webconfig_service, "/config", &webconfig_resource_detail);

HwaHw::HwaHw(mcu::Hwa& mcu)
    : _mcu(mcu)
{}

int HwaHw::start_server(WebConfig& endpoint)
{
    webconfig_resource_detail.user_data = &endpoint;
    return http_server_start();
}

void HwaHw::stop_server()
{
    http_server_stop();
}

int HwaHw::receive(int socket, std::span<uint8_t> buffer, FrameInfo& info)
{
    uint32_t message_type = 0;
    uint64_t remaining    = 0;

    const int received = websocket_recv_msg(socket,
                                            buffer.data(),
                                            buffer.size(),
                                            &message_type,
                                            &remaining,
                                            SYS_FOREVER_MS);

    info.binary    = (message_type & WEBSOCKET_FLAG_BINARY) != 0U;
    info.close     = (message_type & WEBSOCKET_FLAG_CLOSE) != 0U;
    info.remaining = remaining;

    return received;
}

int HwaHw::send(int socket, std::span<const uint8_t> data)
{
    return websocket_send_msg(socket,
                              data.data(),
                              data.size(),
                              WEBSOCKET_OPCODE_DATA_BINARY,
                              false,
                              true,
                              SYS_FOREVER_MS);
}

void HwaHw::unregister(int socket)
{
    websocket_unregister(socket);
}

void HwaHw::reboot_to_bootloader()
{
    _mcu.reboot(fw_selector::FwType::Bootloader);
}

#endif
