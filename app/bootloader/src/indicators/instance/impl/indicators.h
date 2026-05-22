/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "bootloader/src/indicators/shared/deps.h"
#include "bootloader/src/signaling/signaling.h"

#include "zlibs/utils/misc/kwork_delayable.h"

#include <cstdint>

namespace opendeck::bootloader::indicators
{
    /**
     * @brief Bootloader indicator state machine.
     */
    class Indicators
    {
        public:
        /**
         * @brief Constructs the indicator state machine.
         *
         * @param hwa Hardware indicator controls.
         */
        explicit Indicators(Hwa& hwa);

        /**
         * @brief Initializes indicators and turns them on.
         *
         * @return `true` when indicators are available, otherwise `false`.
         */
        bool init();

        /**
         * @brief Starts blinking all indicators.
         */
        void start_blinking_all();

        private:
        static constexpr uint32_t BLINK_INTERVAL_MS = 50U;

        void blink();

        Hwa&                               _hwa;
        zlibs::utils::misc::KworkDelayable _blink_work;
        bool                               _configured = false;
        bool                               _blink_on   = false;
    };
}    // namespace opendeck::bootloader::indicators
