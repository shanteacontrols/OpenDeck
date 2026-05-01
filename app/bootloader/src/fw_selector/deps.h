/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "common.h"

#include <span>

namespace fw_selector
{
    /**
     * @brief Hardware abstraction used to choose and load a firmware image.
     */
    class Hwa
    {
        public:
        virtual ~Hwa() = default;

        /**
         * @brief Returns whether the hardware boot trigger is active.
         *
         * @return `true` when a hardware trigger requests alternate firmware, otherwise `false`.
         */
        virtual bool is_hw_trigger_active() = 0;

        /**
         * @brief Returns whether the software boot trigger is active.
         *
         * @return `true` when a software trigger requests alternate firmware, otherwise `false`.
         */
        virtual bool is_sw_trigger_active() = 0;

        /**
         * @brief Returns the address layout used to validate and start the application.
         *
         * @return Application address layout.
         */
        virtual AppInfo app_info() const = 0;

        /**
         * @brief Reads bytes from the application slot.
         *
         * @param offset Offset from the beginning of the application slot.
         * @param data Destination buffer.
         *
         * @return `true` if the bytes were read, otherwise `false`.
         */
        virtual bool read_app(uint32_t offset, std::span<uint8_t> data) = 0;

        /**
         * @brief Transfers control to the selected firmware image.
         *
         * @param type Firmware image to load.
         */
        virtual void load(FwType type) = 0;
    };
}    // namespace fw_selector
