/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <zephyr/drivers/adc.h>
#include <zephyr/drivers/gpio.h>

#include <cstddef>
#include <cstdint>
#include <optional>

namespace io::analog
{
    /**
     * @brief Base interface for synchronous analog input drivers.
     */
    class DriverBase
    {
        public:
        virtual ~DriverBase() = default;

        /**
         * @brief Initializes the concrete analog driver.
         *
         * @return `true` if initialization succeeded, otherwise `false`.
         */
        virtual bool init() = 0;

        /**
         * @brief Deinitializes the concrete analog driver.
         */
        virtual void deinit() = 0;

        /**
         * @brief Returns the next completed analog frame when available.
         *
         * @return Completed frame, or `std::nullopt` when no frame is ready.
         */
        virtual std::optional<Frame> read() = 0;
    };
}    // namespace io::analog
