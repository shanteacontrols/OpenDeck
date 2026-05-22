/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "common/src/mcu/shared/common.h"
#include "common/src/mcu/shared/deps.h"

#include <array>
#include <optional>

namespace opendeck::common::mcu
{
    /**
     * @brief Test MCU services backed by in-memory state.
     */
    class HwaTest : public opendeck::common::mcu::Hwa
    {
        public:
        /**
         * @brief Returns the configured in-memory serial number.
         *
         * @return Test serial-number bytes.
         */
        std::span<uint8_t> serial_number() override
        {
            return serial;
        }

        /**
         * @brief Records the requested firmware target without rebooting.
         *
         * @param type Firmware target requested by the caller.
         */
        void reboot(opendeck::common::mcu::BootTarget type) override
        {
            reboot_type = type;
        }

        /** @brief Serial-number bytes returned by serial_number(). */
        std::array<uint8_t, opendeck::common::mcu::SERIAL_NUMBER_BUFFER_SIZE> serial = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };

        /** @brief Last firmware target requested through reboot(). */
        std::optional<opendeck::common::mcu::BootTarget> reboot_type = {};
    };
}    // namespace opendeck::common::mcu
