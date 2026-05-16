/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/mcu/shared/common.h"
#include "firmware/src/mcu/shared/deps.h"

#include <array>
#include <optional>

namespace opendeck::mcu
{
    /**
     * @brief Test MCU services backed by in-memory state.
     */
    class HwaTest : public Hwa
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
        void reboot(fw_selector::FwType type) override
        {
            reboot_type = type;
        }

        /** @brief Serial-number bytes returned by serial_number(). */
        std::array<uint8_t, SERIAL_NUMBER_BUFFER_SIZE> serial = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };

        /** @brief Last firmware target requested through reboot(). */
        std::optional<fw_selector::FwType> reboot_type = {};
    };
}    // namespace opendeck::mcu
