/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/io/digital/encoders/instance/impl/deps.h"

#include <array>

namespace opendeck::firmware::io::encoders
{
    /**
     * @brief Test encoder filter that accepts every movement sample.
     */
    class FilterTest : public Filter
    {
        public:
        FilterTest() = default;

        /**
         * @brief Decodes every encoder sample and accepts every resolved movement step.
         *
         * @param index Encoder index being processed.
         * @param pair_state Current two-bit encoder pair state.
         * @param filtered_position Output position after filtering.
         * @param movement_elapsed_time Time since the previous movement, or `std::nullopt` when none exists.
         *
         * @return `true` when a step was decoded, otherwise `false`.
         */
        bool is_filtered(size_t                  index,
                         uint8_t                 pair_state,
                         Position&               filtered_position,
                         std::optional<uint32_t> movement_elapsed_time) override
        {
            auto position = Position::Stopped;
            pair_state &= ENCODER_STATE_MASK;
            bool process = true;

            if (!(_encoder_data[index] & ENCODER_DATA_VALID_MASK))
            {
                process = false;
            }

            _encoder_data[index] <<= 2;
            _encoder_data[index] |= pair_state;
            _encoder_data[index] |= ENCODER_DATA_VALID_MASK;

            if (!process)
            {
                return false;
            }

            _encoder_pulses[index] = static_cast<int8_t>(_encoder_pulses[index] + ENCODER_LOOK_UP_TABLE[_encoder_data[index] & ENCODER_LOOKUP_MASK]);

            if (abs(_encoder_pulses[index]) >= static_cast<int32_t>(Filter::PULSES_PER_STEP))
            {
                position               = (_encoder_pulses[index] > 0) ? Position::Ccw : Position::Cw;
                _encoder_pulses[index] = 0;
            }

            filtered_position = position;

            return position != Position::Stopped;
        }

        /**
         * @brief Resets the decoder state for one encoder.
         *
         * @param index Encoder index to reset.
         */
        void reset(size_t index) override
        {
            _encoder_data[index]   = 0;
            _encoder_pulses[index] = 0;
        }

        private:
        static constexpr size_t  STORAGE_SIZE              = Collection::size() ? Collection::size() : 1;
        static constexpr uint8_t ENCODER_STATE_MASK        = 0x03;
        static constexpr uint8_t ENCODER_DATA_VALID_MASK   = 0x80;
        static constexpr uint8_t ENCODER_LOOKUP_MASK       = 0x0F;
        static constexpr int8_t  ENCODER_LOOK_UP_TABLE[16] = {
            0, 1, -1, 0, -1, 0, 0, 1, 1, 0, 0, -1, 0, -1, 1, 0
        };

        std::array<uint8_t, STORAGE_SIZE> _encoder_data   = {};
        std::array<int8_t, STORAGE_SIZE>  _encoder_pulses = {};
    };
}    // namespace opendeck::firmware::io::encoders
