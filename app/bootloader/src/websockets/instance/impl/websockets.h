/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "bootloader/src/direct_update_writer/instance/impl/direct_update_writer.h"
#include "common/src/websockets/firmware_upload/firmware_upload.h"
#include "common/src/websockets/shared/deps.h"
#include "common/src/websockets/shared/firmware_upload.h"

#include "zlibs/utils/misc/mutex.h"
#include "zlibs/utils/threads/threads.h"

#include <zephyr/kernel.h>

#include <array>
#include <atomic>
#include <cstdint>
#include <optional>
#include <span>

namespace opendeck::bootloader::websockets
{
    /**
     * @brief Minimal bootloader WebSockets endpoint for network DFU.
     */
    class WebSockets : public opendeck::websockets::Endpoint
    {
        public:
        /**
         * @brief Constructs the endpoint around platform hooks and direct update storage.
         *
         * @param hwa Platform hooks used for WebSocket I/O.
         * @param direct_update_writer Writer that installs validated firmware payloads.
         */
        WebSockets(opendeck::websockets::Hwa& hwa, direct_update_writer::DirectUpdateWriter& direct_update_writer);
        ~WebSockets();

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
        using ClientThread = zlibs::utils::threads::UserThread<zlibs::utils::misc::StringLiteral{ "boot_websocket" },
                                                               K_PRIO_PREEMPT(1),
                                                               4096>;

        opendeck::websockets::Hwa&                                            _hwa;
        opendeck::websockets::FirmwareUpload                                  _firmware_upload;
        k_sem                                                                 _client_wakeup  = {};
        std::atomic<int>                                                      _client_socket  = -1;
        std::atomic<bool>                                                     _shutdown       = false;
        bool                                                                  _server_running = false;
        ClientThread                                                          _client_thread;
        zlibs::utils::misc::Mutex                                             _client_state_lock;
        std::array<uint8_t, opendeck::websockets::FIRMWARE_UPLOAD_FRAME_SIZE> _rx_buffer = {};

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
         * @brief Processes one WebSockets command frame.
         *
         * @param data Frame payload bytes received from the active client.
         *
         * @return ACK frame when a command was handled, otherwise `std::nullopt`.
         */
        std::optional<opendeck::websockets::FirmwareUploadAck> handle_command_frame(std::span<const uint8_t> data);
    };
}    // namespace opendeck::bootloader::websockets
