/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "bootloader/src/protocols/webusb/shared/common.h"

#include <functional>
#include <span>
#include <string_view>
#include <cstdint>

namespace opendeck::bootloader::protocols::webusb
{
    /**
     * @brief Platform hooks used by the bootloader WebUSB endpoint.
     */
    class Hwa
    {
        public:
        using RxCallback              = std::function<bool(DfuRxChunk)>;
        using ConnectionStateCallback = std::function<void(bool connected)>;

        virtual ~Hwa() = default;

        /**
         * @brief Initializes WebUSB.
         *
         * @return `true` on success, otherwise `false`.
         */
        virtual bool init() = 0;

        /**
         * @brief Deinitializes WebUSB.
         *
         * @return `true` on success, otherwise `false`.
         */
        virtual bool deinit() = 0;

        /**
         * @brief Sends one device-side status line to the host.
         *
         * @param message Status string to send.
         */
        virtual void status(std::string_view message) = 0;

        /**
         * @brief Registers a callback invoked for each received WebUSB OUT buffer.
         *
         * @param callback Callback to invoke. Passing an empty callback disables delivery.
         */
        virtual void register_rx_callback(RxCallback callback) = 0;

        /**
         * @brief Releases a previously accepted WebUSB OUT buffer back to the hardware adapter.
         *
         * @param chunk Received buffer to release.
         */
        virtual void release_rx_buffer(DfuRxChunk chunk) = 0;

        /**
         * @brief Registers a callback invoked when the WebUSB connection state changes.
         *
         * @param callback Callback to invoke. Passing an empty callback disables notifications.
         */
        virtual void register_connection_state_callback(ConnectionStateCallback callback) = 0;
    };
}    // namespace opendeck::bootloader::protocols::webusb
