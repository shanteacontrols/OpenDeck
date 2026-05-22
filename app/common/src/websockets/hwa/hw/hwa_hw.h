/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "common/src/websockets/shared/deps.h"

#include <zephyr/kernel.h>
#include <zephyr/net/http/server.h>
#include <zephyr/net/http/service.h>
#include <zephyr/net/websocket.h>

#include <cerrno>

namespace opendeck::websockets
{
    /**
     * @brief Zephyr-backed WebSocket transport hooks.
     *
     * Endpoint-specific modules still define their HTTP service/resource storage
     * because Zephyr's service macros require unique static symbols and paths.
     */
    class HwaHw : public Hwa
    {
        public:
        /**
         * @brief Constructs Zephyr-backed WebSocket hooks.
         *
         * @param resource_detail WebSocket resource detail owned by the endpoint module.
         */
        explicit HwaHw(http_resource_detail_websocket& resource_detail)
            : _resource_detail(resource_detail)
        {}

        int start_server(Endpoint& endpoint) override
        {
            _resource_detail.user_data = &endpoint;
            return http_server_start();
        }

        void stop_server() override
        {
            http_server_stop();
        }

        int receive(int socket, std::span<uint8_t> buffer, FrameInfo& info) override
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

        int send(int socket, std::span<const uint8_t> data) override
        {
            return websocket_send_msg(socket,
                                      data.data(),
                                      data.size(),
                                      WEBSOCKET_OPCODE_DATA_BINARY,
                                      false,
                                      true,
                                      SYS_FOREVER_MS);
        }

        void unregister(int socket) override
        {
            websocket_unregister(socket);
        }

        /**
         * @brief Accepts a WebSocket upgrade and hands the socket to the endpoint.
         *
         * @param ws_socket Upgraded WebSocket socket descriptor.
         * @param request_ctx HTTP request context supplied by Zephyr.
         * @param user_data Endpoint instance attached to the resource.
         *
         * @return 0 on success, otherwise a negative errno value.
         */
        static int setup(int                                       ws_socket,
                         [[maybe_unused]] struct http_request_ctx* request_ctx,
                         void*                                     user_data)
        {
            auto self = static_cast<Endpoint*>(user_data);

            if (self == nullptr)
            {
                return -EINVAL;
            }

            return self->accept_client(ws_socket);
        }

        private:
        http_resource_detail_websocket& _resource_detail;
    };
}    // namespace opendeck::websockets
