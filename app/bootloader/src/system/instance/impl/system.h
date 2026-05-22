/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "bootloader/src/system/shared/deps.h"

namespace opendeck::bootloader::system
{
    /**
     * @brief Top-level bootloader coordinator.
     */
    class System
    {
        public:
        /**
         * @brief Constructs the bootloader coordinator.
         *
         * @param hwa Platform services used by the bootloader.
         */
        explicit System(Hwa& hwa);

        /**
         * @brief Initializes staged update handling, indicators, and recovery transports.
         *
         * @return `true` if initialization completed, otherwise `false`.
         */
        bool init();

        private:
        Hwa& _hwa;
    };
}    // namespace opendeck::bootloader::system
