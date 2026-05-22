/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <string_view>

namespace opendeck::webusb
{
    /**
     * @brief Platform hooks used by the bootloader WebUSB endpoint.
     */
    class Hwa
    {
        public:
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
    };
}    // namespace opendeck::webusb
