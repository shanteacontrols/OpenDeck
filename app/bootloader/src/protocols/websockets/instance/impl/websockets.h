/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "bootloader/src/threads.h"
#include "common/src/protocols/websockets/handler/handler.h"
#include "common/src/protocols/websockets/shared/deps.h"
#include "common/src/protocols/websockets/shared/firmware_upload.h"

#include "zlibs/utils/misc/mutex.h"

#include <zephyr/kernel.h>

#include <array>
#include <atomic>
#include <cstdint>
#include <optional>
#include <span>

namespace opendeck::bootloader::protocols::websockets
{
    /**
     * @brief Minimal bootloader WebSockets endpoint for network DFU.
     */
    class WebSockets : public opendeck::common::protocols::websockets::Endpoint
    {
        public:
        /**
         * @brief Constructs the endpoint around platform hooks.
         *
         * @param hwa Platform hooks used for WebSocket I/O.
         */
        explicit WebSockets(opendeck::common::protocols::websockets::Hwa& hwa);
        ~WebSockets() override;

        /**
         * @brief Starts the network DFU endpoint.
         *
         * @return `true` on success, otherwise `false`.
         */
        bool init() override;

        /**
         * @brief Stops the network DFU endpoint.
         *
         * @return `true` on success.
         */
        bool deinit();

        /**
         * @brief Accepts a WebSocket client handed off by Zephyr's HTTP server.
         *
         * @param socket WebSocket socket descriptor.
         *
         * @return 0 on success, otherwise a negative errno value.
         */
        int accept_client(int socket) override;

        private:
        opendeck::common::protocols::websockets::Hwa&                                            _hwa;
        k_sem                                                                                    _client_wakeup  = {};
        std::atomic<int>                                                                         _client_socket  = -1;
        std::atomic<bool>                                                                        _shutdown       = false;
        bool                                                                                     _server_running = false;
        threads::WebSocketsClientThread                                                          _client_thread;
        zlibs::utils::misc::Mutex                                                                _client_state_lock;
        std::array<uint8_t, opendeck::common::protocols::websockets::FIRMWARE_UPLOAD_FRAME_SIZE> _rx_buffer = {};

        /**
         * @brief Runs the active-client receive loop.
         */
        void client_loop();

        /**
         * @brief Stops the server and worker thread.
         */
        bool stop();

        /**
         * @brief Closes the active WebSocket client, if one is connected.
         */
        void close_client();

        /**
         * @brief Resets state owned by registered WebSockets handlers.
         */
        void reset_handlers();

        /**
         * @brief Processes one WebSockets command frame.
         *
         * @param data Frame payload bytes received from the active client.
         *
         * @return ACK frame when a command was handled, otherwise `std::nullopt`.
         */
        std::optional<std::span<const uint8_t>> handle_command_frame(std::span<const uint8_t> data);
    };
}    // namespace opendeck::bootloader::protocols::websockets
