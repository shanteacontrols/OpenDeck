/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "bootloader/src/system/hwa/hw/hwa_hw.h"
#include "bootloader/src/system/instance/impl/system.h"

namespace opendeck::bootloader::system
{
    /**
     * @brief Convenience builder that wires the bootloader system.
     */
    class Builder
    {
        public:
        Builder()
            : _instance(_hwa)
        {}

        /**
         * @brief Returns the bootloader system instance.
         *
         * @return Bootloader system instance.
         */
        System& instance()
        {
            return _instance;
        }

        private:
        HwaHw  _hwa;
        System _instance;
    };
}    // namespace opendeck::bootloader::system
