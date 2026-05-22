/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

namespace opendeck::bootloader::io::indicators
{
    /**
     * @brief No-op bootloader indicators used when a target has no indicator hardware.
     */
    class IndicatorsStub
    {
        public:
        /**
         * @brief Initializes no-op indicators.
         *
         * @return Always `true`.
         */
        bool init()
        {
            return true;
        }

        /**
         * @brief Starts no-op blinking.
         */
        void start_blinking_all()
        {}
    };
}    // namespace opendeck::bootloader::io::indicators
