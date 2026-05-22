/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cstdint>
#include <span>

namespace opendeck::common::protocols::websockets
{
    /**
     * @brief OpenDeck WebSocket endpoint.
     */
    class Endpoint
    {
        public:
        virtual ~Endpoint() = default;

        /**
         * @brief Initializes the WebSocket endpoint.
         *
         * @return `true` on success, otherwise `false`.
         */
        virtual bool init() = 0;

        /**
         * @brief Accepts an upgraded WebSocket client.
         *
         * @param socket WebSocket socket descriptor.
         *
         * @return 0 on success, otherwise a negative errno value.
         */
        virtual int accept_client(int socket) = 0;
    };

    /**
     * @brief Metadata for one received WebSocket frame.
     */
    struct FrameInfo
    {
        bool     binary    = false;
        bool     close     = false;
        uint64_t remaining = 0;
    };

    /**
     * @brief Platform hooks for one OpenDeck WebSocket endpoint.
     */
    class Hwa
    {
        public:
        virtual ~Hwa() = default;

        /**
         * @brief Starts the WebSocket server.
         *
         * @param endpoint Endpoint that accepts upgraded clients.
         *
         * @return 0 on success, otherwise a negative errno value.
         */
        virtual int start_server(Endpoint& endpoint) = 0;

        /**
         * @brief Stops the WebSocket server.
         */
        virtual void stop_server() = 0;

        /**
         * @brief Receives one WebSocket frame.
         *
         * @param socket Active WebSocket socket.
         * @param buffer Buffer used for frame bytes.
         * @param info Frame metadata populated on success.
         *
         * @return Received byte count on success, otherwise a negative errno value.
         */
        virtual int receive(int socket, std::span<uint8_t> buffer, FrameInfo& info) = 0;

        /**
         * @brief Sends one binary WebSocket frame.
         *
         * @param socket Active WebSocket socket.
         * @param data Frame payload bytes.
         *
         * @return Sent byte count on success, otherwise a negative errno value.
         */
        virtual int send(int socket, std::span<const uint8_t> data) = 0;

        /**
         * @brief Closes one WebSocket socket.
         *
         * @param socket Socket to close.
         */
        virtual void unregister(int socket) = 0;
    };
}    // namespace opendeck::common::protocols::websockets
