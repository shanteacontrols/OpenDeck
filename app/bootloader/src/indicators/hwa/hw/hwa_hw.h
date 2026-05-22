/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "bootloader/src/indicators/shared/deps.h"
#include "common/src/indicators/hwa/hw/hwa_hw.h"
#include "common/src/indicators/shared/common.h"

namespace opendeck::bootloader::indicators
{
    /**
     * @brief Hardware-backed bootloader indicators.
     */
    class HwaHw : public Hwa
    {
        public:
        /**
         * @brief Initializes common indicator hardware.
         *
         * @return `true` when indicators are available, otherwise `false`.
         */
        bool init() override
        {
            return _hwa.init();
        }

        /**
         * @brief Turns all indicators on.
         */
        void on() override
        {
            _hwa.on(io::indicators::Type::All);
        }

        /**
         * @brief Turns all indicators off.
         */
        void off() override
        {
            _hwa.off(io::indicators::Type::All);
        }

        private:
        io::indicators::HwaHw _hwa;
    };
}    // namespace opendeck::bootloader::indicators
