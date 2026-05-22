/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

namespace opendeck::bootloader::system
{
    /**
     * @brief Platform services owned by the bootloader system coordinator.
     */
    class Hwa
    {
        public:
        virtual ~Hwa() = default;

        /**
         * @brief Installs a staged update when one is present.
         *
         * @return `true` if a staged update was consumed, otherwise `false`.
         */
        virtual bool consume_staged_update() = 0;

        /**
         * @brief Reboots into the main application image.
         */
        virtual void reboot_application() = 0;

        /**
         * @brief Initializes the bootloader indicators.
         */
        virtual void init_indicators() = 0;

        /**
         * @brief Initializes the WebUSB recovery transport.
         *
         * @return `true` if WebUSB is disabled or initialized successfully, otherwise `false`.
         */
        virtual bool init_webusb() = 0;

        /**
         * @brief Initializes network discovery for recovery transports.
         *
         * @return `true` if mDNS is disabled or initialized successfully, otherwise `false`.
         */
        virtual bool init_mdns() = 0;
    };
}    // namespace opendeck::bootloader::system
