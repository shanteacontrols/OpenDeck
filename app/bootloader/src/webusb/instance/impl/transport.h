/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

namespace opendeck::webusb
{
    /**
     * @brief Sends one device-side status line to the host over WebUSB when an IN endpoint is active.
     *
     * The message is best-effort. If the host is not connected yet or no IN buffer
     * can be allocated, the status line is silently dropped.
     *
     * @param message Null-terminated status string to send.
     */
    void status(const char* message);

    /**
     * @brief Bootloader WebUSB transport that feeds incoming DFU bytes into the updater.
     */
    class Transport
    {
        public:
        /**
         * @brief Initializes the bootloader WebUSB transport.
         *
         * @return `true` on success, otherwise `false`.
         */
        bool init();

        /**
         * @brief Deinitializes the bootloader WebUSB transport.
         *
         * @return `true` on success, otherwise `false`.
         */
        bool deinit();
    };
}    // namespace opendeck::webusb
