/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "deps.h"

#include <array>

namespace opendeck::io::encoders
{
    /**
     * @brief Debounces encoder direction changes by requiring repeated movement in the same direction.
     */
    class FilterHw : public Filter
    {
        public:
        FilterHw() = default;

        /**
         * @brief Filters one sampled encoder movement.
         *
         * @param index Encoder index being filtered.
         * @param position Raw movement direction.
         * @param filtered_position Output storage for the filtered direction.
         * @param sample_taken_time Sample timestamp in milliseconds.
         *
         * @return `true` when the sample was processed, otherwise `false`.
         */
        bool is_filtered(size_t    index,
                         Position  position,
                         Position& filtered_position,
                         uint32_t  sample_taken_time) override
        {
            filtered_position = position;

            // disable debouncing mode if encoder isn't moving for more than
            // ENCODERS_DEBOUNCE_RESET_TIME milliseconds
            if ((sample_taken_time - _last_movement_time[index]) > ENCODERS_DEBOUNCE_RESET_TIME_MS)
            {
                reset(index);
            }

            if (position != Position::Stopped)
            {
                if (_debounce_counter[index] != ENCODERS_DEBOUNCE_COUNT)
                {
                    if (position != _last_direction[index])
                    {
                        _debounce_counter[index] = 0;
                    }

                    _debounce_counter[index]++;

                    if (_debounce_counter[index] == ENCODERS_DEBOUNCE_COUNT)
                    {
                        _debounce_counter[index]   = 0;
                        _debounce_direction[index] = position;
                    }
                }

                _last_direction[index]     = position;
                _last_movement_time[index] = sample_taken_time;

                if (_debounce_direction[index] != Position::Stopped)
                {
                    filtered_position = _debounce_direction[index];
                }

                return true;
            }

            return false;
        }

        /**
         * @brief Clears the debounce state for one encoder.
         *
         * @param index Encoder index to reset.
         */
        void reset(size_t index) override
        {
            _debounce_counter[index]   = 0;
            _debounce_direction[index] = Position::Stopped;
        }

        /**
         * @brief Returns the timestamp of the last non-stopped movement sample.
         *
         * @param index Encoder index to query.
         *
         * @return Timestamp of the last recorded movement in milliseconds.
         */
        uint32_t last_movement_time(size_t index) override
        {
            return _last_movement_time[index];
        }

        private:
        static constexpr uint32_t ENCODERS_DEBOUNCE_RESET_TIME_MS = 50;
        static constexpr uint8_t  ENCODERS_DEBOUNCE_COUNT         = 4;
        static constexpr size_t   STORAGE_SIZE                    = Collection::size() ? Collection::size() : 1;

        std::array<Position, STORAGE_SIZE> _last_direction     = {};
        std::array<Position, STORAGE_SIZE> _debounce_direction = {};
        std::array<uint8_t, STORAGE_SIZE>  _debounce_counter   = {};
        std::array<uint32_t, STORAGE_SIZE> _last_movement_time = {};
    };
}    // namespace opendeck::io::encoders
