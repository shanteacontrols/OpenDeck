/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "driver_base.h"

#include <algorithm>

namespace io::analog::drivers
{
    /**
     * @brief Template base for analog drivers that synchronously scan full frames.
     *
     * @tparam T Concrete driver type that provides the hardware-specific hooks.
     */
    template<typename T>
    class ScanDriverBase : public DriverBase
    {
        public:
        /**
         * @brief Constructs the scan driver base.
         */
        ScanDriverBase() = default;

        ~ScanDriverBase() override = default;

        /**
         * @brief Initializes the concrete driver.
         *
         * @return `true` if initialization succeeded, otherwise `false`.
         */
        bool init() override
        {
            if (!static_cast<T*>(this)->init_driver())
            {
                return false;
            }

            _initialized = true;
            return true;
        }

        /**
         * @brief Stops synchronous scanning.
         */
        void deinit() override
        {
            _initialized = false;
        }

        /**
         * @brief Performs one full synchronous frame sweep.
         *
         * Each selected input is sampled twice and only the second conversion is stored. The
         * first conversion is intentionally discarded so all synchronous scan drivers share the
         * same post-switch settling behavior.
         *
         * @return Completed frame, or `std::nullopt` when the sweep failed.
         */
        std::optional<Frame> read() override
        {
            if (!_initialized)
            {
                return {};
            }

            if (static_cast<T*>(this)->physical_input_count() == 0)
            {
                return {};
            }

            Frame    frame      = {};
            uint16_t min_sample = UINT16_MAX;
            uint16_t max_sample = 0;

            for (size_t i = 0; i < static_cast<T*>(this)->physical_input_count(); i++)
            {
                uint16_t sample = 0;

                static_cast<T*>(this)->select_input(i);

                if (!static_cast<T*>(this)->read_sample(sample))
                {
                    return {};
                }

                if (!static_cast<T*>(this)->read_sample(sample))
                {
                    return {};
                }

                frame[i]   = sample;
                min_sample = std::min(min_sample, sample);
                max_sample = std::max(max_sample, sample);
            }

            return frame;
        }

        private:
        bool _initialized = false;
    };
}    // namespace io::analog::drivers
