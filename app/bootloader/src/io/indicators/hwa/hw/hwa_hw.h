/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "bootloader/src/io/indicators/instance/impl/deps.h"
#include "common/src/io/indicators/hwa/hw/hwa_hw.h"
#include "common/src/io/indicators/shared/common.h"

namespace opendeck::bootloader::io::indicators
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
            _hwa.on(opendeck::common::io::indicators::Type::All);
        }

        /**
         * @brief Turns all indicators off.
         */
        void off() override
        {
            _hwa.off(opendeck::common::io::indicators::Type::All);
        }

        private:
        opendeck::common::io::indicators::HwaHw _hwa;
    };
}    // namespace opendeck::bootloader::io::indicators
