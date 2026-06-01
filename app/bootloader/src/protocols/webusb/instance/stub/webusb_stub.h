/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

namespace opendeck::bootloader::protocols::webusb
{
    /**
     * @brief Stub WebUSB endpoint used when WebUSB recovery is disabled.
     */
    class WebUsb
    {
        public:
        WebUsb() = default;

        /**
         * @brief Initializes the disabled WebUSB endpoint.
         *
         * @return Always `true`.
         */
        bool init()
        {
            return true;
        }

        /**
         * @brief Deinitializes the disabled WebUSB endpoint.
         *
         * @return Always `true`.
         */
        bool deinit()
        {
            return true;
        }
    };
}    // namespace opendeck::bootloader::protocols::webusb
