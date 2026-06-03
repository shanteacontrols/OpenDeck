/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

namespace opendeck::bootloader::io::indicators
{
    /**
     * @brief Hardware indicator controls used by the bootloader.
     */
    class Hwa
    {
        public:
        virtual ~Hwa() = default;

        /**
         * @brief Initializes the indicator hardware.
         *
         * @return `true` when indicators are available, otherwise `false`.
         */
        virtual bool init() = 0;

        /**
         * @brief Turns all bootloader indicators on.
         */
        virtual void on() = 0;

        /**
         * @brief Turns all bootloader indicators off.
         */
        virtual void off() = 0;
    };
}    // namespace opendeck::bootloader::io::indicators
