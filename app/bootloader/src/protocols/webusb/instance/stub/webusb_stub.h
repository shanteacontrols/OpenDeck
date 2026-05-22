/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "bootloader/src/protocols/webusb/shared/deps.h"

namespace opendeck::bootloader::protocols::webusb
{
    /**
     * @brief Stub WebUSB endpoint used when WebUSB recovery is disabled.
     */
    class WebUsbStub : public Hwa
    {
        public:
        WebUsbStub() = default;

        /**
         * @brief Initializes the disabled WebUSB endpoint.
         *
         * @return Always `true`.
         */
        bool init() override
        {
            return true;
        }

        /**
         * @brief Deinitializes the disabled WebUSB endpoint.
         *
         * @return Always `true`.
         */
        bool deinit() override
        {
            return true;
        }

        /**
         * @brief Ignores one status message.
         *
         * @param message Null-terminated status string.
         */
        void status(std::string_view) override
        {}
    };
}    // namespace opendeck::bootloader::protocols::webusb
