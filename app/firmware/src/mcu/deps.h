/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "fw_selector/common.h"

#include <cstdint>
#include <span>

namespace opendeck::mcu
{
    /**
     * @brief MCU-level hardware services shared by firmware subsystems.
     */
    class Hwa
    {
        public:
        virtual ~Hwa() = default;

        /**
         * @brief Reads the hardware serial number bytes.
         *
         * @return View into HWA-owned serial number bytes, or an empty span when unavailable.
         */
        virtual std::span<uint8_t> serial_number() = 0;

        /**
         * @brief Reboots into the requested firmware target.
         *
         * @param type Firmware target to reboot into.
         */
        virtual void reboot(fw_selector::FwType type) = 0;
    };
}    // namespace opendeck::mcu
