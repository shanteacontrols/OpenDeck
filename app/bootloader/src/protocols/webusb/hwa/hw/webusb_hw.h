/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "bootloader/src/protocols/webusb/instance/impl/deps.h"
#include <span>

namespace opendeck::bootloader::protocols::webusb
{
    struct WebUsbHwAccess;

    /**
     * @brief Bootloader WebUSB hardware adapter.
     */
    class WebUsbHw : public Hwa
    {
        public:
        /**
         * @brief Constructs the WebUSB hardware adapter.
         */
        WebUsbHw() = default;

        /**
         * @brief Initializes bootloader WebUSB.
         *
         * @return `true` on success, otherwise `false`.
         */
        bool init() override;

        /**
         * @brief Deinitializes bootloader WebUSB.
         *
         * @return `true` on success, otherwise `false`.
         */
        bool deinit() override;

        /**
         * @brief Sends one device-side status line to the host.
         *
         * @param message Null-terminated status string to send.
         */
        void status(std::string_view message) override;

        /**
         * @brief Stores the WebUSB RX callback provided by the instance layer.
         *
         * The hardware adapter invokes this callback for each received USB OUT buffer.
         *
         * @param callback Callback to invoke for incoming WebUSB OUT buffers.
         */
        void register_rx_callback(RxCallback callback) override;

        /**
         * @brief Releases a previously accepted WebUSB OUT buffer.
         *
         * @param chunk Buffer to release back to USB.
         */
        void release_rx_buffer(DfuRxChunk chunk) override;

        /**
         * @brief Stores the WebUSB connection-state callback provided by the instance layer.
         *
         * The hardware adapter invokes this callback when the USB function is enabled or disabled so
         * the instance layer can reset upload-session state.
         *
         * @param callback Callback to invoke on WebUSB connect/disconnect state changes.
         */
        void register_connection_state_callback(ConnectionStateCallback callback) override;

        private:
        friend struct WebUsbHwAccess;

        RxCallback              _rx_callback               = {};
        ConnectionStateCallback _connection_state_callback = {};

        /**
         * @brief Delivers one received WebUSB DFU buffer to the instance layer.
         *
         * @param chunk Upload-command buffer and owning USB class data.
         *
         * @return `true` when the buffer was accepted, otherwise `false`.
         */
        bool feed(DfuRxChunk chunk);
    };
}    // namespace opendeck::bootloader::protocols::webusb
