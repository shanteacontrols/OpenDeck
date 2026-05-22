/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "bootloader/src/installer/hwa/hw/hwa_hw.h"
#include "bootloader/src/installer/instance/impl/installer.h"
#include "common/src/mcu/shared/deps.h"

namespace opendeck::installer
{
    /**
     * @brief Convenience builder that wires the hardware installer backend.
     */
    class Builder
    {
        public:
        /**
         * @brief Constructs the installer builder.
         *
         * @param mcu Shared MCU services.
         */
        explicit Builder(mcu::Hwa& mcu)
            : _hwa(mcu)
            , _instance(_hwa)
        {}

        /**
         * @brief Returns the constructed installer instance.
         *
         * @return Hardware-backed installer instance.
         */
        Installer& instance()
        {
            return _instance;
        }

        private:
        HwaHw     _hwa;
        Installer _instance;
    };
}    // namespace opendeck::installer
