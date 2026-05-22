/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "bootloader/src/signaling/signaling.h"
#include "bootloader/src/protocols/webusb/shared/deps.h"

namespace opendeck::bootloader::protocols::webusb
{
    /**
     * @brief Bootloader WebUSB endpoint.
     */
    class WebUsb
    {
        public:
        /**
         * @brief Constructs the WebUSB endpoint.
         *
         * @param hwa Platform hooks used by WebUSB.
         */
        explicit WebUsb(Hwa& hwa);

        /**
         * @brief Initializes WebUSB.
         *
         * @return `true` on success, otherwise `false`.
         */
        bool init();

        /**
         * @brief Deinitializes WebUSB.
         *
         * @return `true` on success, otherwise `false`.
         */
        bool deinit();

        private:
        Hwa& _hwa;
    };
}    // namespace opendeck::bootloader::protocols::webusb
