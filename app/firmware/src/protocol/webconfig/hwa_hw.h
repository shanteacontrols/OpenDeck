/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "deps.h"
#include "mcu/deps.h"

namespace opendeck::protocol::webconfig
{
    /**
     * @brief Zephyr-backed WebConfig platform hooks.
     */
    class HwaHw : public Hwa
    {
        public:
        explicit HwaHw(mcu::Hwa& mcu);

        /**
         * @brief Starts Zephyr's HTTP server for the WebConfig endpoint.
         *
         * @param endpoint Endpoint that accepts upgraded clients.
         *
         * @return 0 on success, otherwise a negative errno value.
         */
        int start_server(WebConfig& endpoint) override;

        /**
         * @brief Stops Zephyr's HTTP server.
         */
        void stop_server() override;

        /**
         * @brief Receives one WebSocket frame.
         *
         * @param socket Active WebSocket socket.
         * @param buffer Buffer used for frame bytes.
         * @param info Frame metadata populated on success.
         *
         * @return Received byte count on success, otherwise a negative errno value.
         */
        int receive(int socket, std::span<uint8_t> buffer, FrameInfo& info) override;

        /**
         * @brief Sends one binary WebSocket frame.
         *
         * @param socket Active WebSocket socket.
         * @param data Frame payload bytes.
         *
         * @return Sent byte count on success, otherwise a negative errno value.
         */
        int send(int socket, std::span<const uint8_t> data) override;

        /**
         * @brief Closes one WebSocket socket.
         *
         * @param socket Socket to close.
         */
        void unregister(int socket) override;

        /**
         * @brief Reboots into the bootloader so a staged update can be applied.
         */
        void reboot_to_bootloader() override;

        private:
        mcu::Hwa& _mcu;
    };
}    // namespace opendeck::protocol::webconfig
