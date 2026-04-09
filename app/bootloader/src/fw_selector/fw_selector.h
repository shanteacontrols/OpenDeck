/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "common.h"
#include "deps.h"

namespace fw_selector
{
    /**
     * @brief Chooses which firmware image should be started.
     */
    class FwSelector
    {
        public:
        /**
         * @brief Constructs the selector around one hardware-abstraction instance.
         *
         * @param hwa Hardware abstraction used to inspect triggers and image validity.
         */
        explicit FwSelector(Hwa& hwa)
            : _hwa(hwa)
        {}

        /**
         * @brief Selects the firmware image and trigger reason to use.
         *
         * @return Firmware-selection result.
         */
        Selection select();

        private:
        Hwa& _hwa;
    };
}    // namespace fw_selector
