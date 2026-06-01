/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "common/src/protocols/websockets/handler/handler.h"
#include "common/src/protocols/websockets/shared/buffers.h"
#include "common/src/protocols/websockets/shared/deps.h"
#include "common/src/threads.h"

#include "zlibs/utils/misc/mutex.h"

#include <zephyr/kernel.h>

#include <atomic>
#include <cstdint>
#include <span>

namespace opendeck::common::protocols::websockets
{
    /**
     * @brief Shared WebSocket endpoint implementation.
     */
    class BaseWebSockets : public Endpoint, private HandlerEndpoint
    {
        public:
        /**
         * @brief Constructs a shared WebSocket endpoint.
         *
         * @param hwa Platform hooks used for WebSocket I/O.
         */
        explicit BaseWebSockets(Hwa& hwa);
        ~BaseWebSockets() override;

        /**
         * @brief Starts the WebSocket server and worker threads.
         *
         * @return `true` on success, otherwise `false`.
         */
        bool init() override;

        /**
         * @brief Stops the WebSocket server and worker threads.
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

        /**
         * @brief Returns RX/TX storage used by the endpoint.
         *
         * @return RX/TX storage used by the endpoint.
         */
        virtual BuffersBase& buffers() = 0;

        private:
        Hwa&                        _hwa;
        k_sem                       _client_wakeup        = {};
        k_sem                       _tx_wakeup            = {};
        std::atomic<int>            _client_socket        = -1;
        uint32_t                    _client_generation    = 0;
        std::atomic<bool>           _shutdown             = false;
        std::atomic<bool>           _server_running       = false;
        bool                        _handlers_initialized = false;
        threads::WebSocketsThread   _client_thread;
        threads::WebSocketsTxThread _tx_thread;
        zlibs::utils::misc::Mutex   _send_mutex;
        zlibs::utils::misc::Mutex   _client_state_lock;

        /**
         * @brief Worker loop that receives binary WebSocket frames and routes them by frame type.
         */
        void client_loop();

        /**
         * @brief Worker loop that sends queued binary WebSocket frames.
         */
        void tx_loop();

        /**
         * @brief Stops the WebSockets server and worker threads.
         */
        bool stop();

        /**
         * @brief Closes whichever WebSocket client is currently active.
         *
         * Used during shutdown when there is no receive-loop socket/session pair to validate.
         * If a client is connected, this clears queued TX data and notifies handlers that the
         * captured session closed.
         */
        void close_client();

        /**
         * @brief Closes a specific WebSocket client if socket and generation still match.
         *
         * Used when replacing a client or when a receive loop exits. The socket/generation
         * guard prevents stale receive loops from closing a newer active client.
         *
         * @param socket Socket descriptor expected to be the active client.
         * @param generation Client generation expected to be active.
         *
         * @return True when the client was closed, otherwise false.
         */
        bool close_client(int socket, uint32_t generation);

        /**
         * @brief Drops queued transmit data for disconnected clients.
         */
        void clear_tx();

        /**
         * @brief Notifies handlers that a WebSockets client session closed.
         *
         * @param session_id Closed client session id.
         */
        void close_session(uint32_t session_id);

        /**
         * @brief Processes one WebSockets command frame.
         *
         * @param data Frame payload bytes received from the active client.
         * @param session_id Client generation associated with the received frame.
         */
        void handle_command_frame(std::span<const uint8_t> data, uint32_t session_id);

        /**
         * @brief Initializes all registered WebSockets handlers.
         */
        void init_handlers();

        /**
         * @brief Checks whether a WebSockets client session is still active.
         *
         * @param session_id Client generation to validate.
         *
         * @return `true` if the session is still the active client.
         */
        bool session_active(uint32_t session_id) override;

        /**
         * @brief Queues one handler frame for a specific client session.
         *
         * @param data Complete frame payload.
         * @param session_id Client generation that must still be active.
         */
        void queue_frame(std::span<const uint8_t> data, uint32_t session_id) override;

        /**
         * @brief Queues one handler frame for the active WebSockets client.
         *
         * @param data Complete frame payload.
         */
        void queue_frame(std::span<const uint8_t> data) override;

        /**
         * @brief Sends queued binary WebSocket frames from worker context.
         */
        void process_tx_queue();
    };
}    // namespace opendeck::common::protocols::websockets
