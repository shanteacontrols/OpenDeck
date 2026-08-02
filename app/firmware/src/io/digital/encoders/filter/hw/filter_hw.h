/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/io/digital/encoders/instance/impl/deps.h"

#include "zlibs/utils/misc/bit.h"

#include <array>

namespace opendeck::firmware::io::encoders
{
    /**
     * @brief Hardware-oriented quadrature decoder and encoder direction-change filter.
     *
     * Valid A/B transitions are accumulated until they form one complete encoder step. Completed
     * steps are then passed through direction-change filtering: once a direction is established,
     * four consecutive steps in the opposite direction are required before the reported direction
     * changes. Direction history expires after an idle period, while quadrature decoding state is
     * preserved so the first transition of resumed movement is not discarded.
     */
    class FilterHw : public Filter
    {
        public:
        FilterHw() = default;

        /**
         * @brief Decodes and filters one sampled encoder movement.
         *
         * Each encoder sample moves through the following states:
         * - Step 1: mask the sample to the two encoder A/B state bits
         * - Step 2: if no previous A/B state exists, store the current state as the decoder seed
         *           and suppress the sample
         * - Step 3: combine the previous and current A/B states, use the quadrature lookup table
         *           to obtain a `-1`, `0`, or `+1` transition, and add it to the pulse accumulator
         * - Step 4: suppress the sample until the accumulator reaches one complete encoder step;
         *           then convert its sign into a raw clockwise/counter-clockwise direction and
         *           clear the pulse accumulator
         * - Step 5: if this is the first movement or movement resumed after the idle timeout,
         *           clear only direction-debounce history while preserving quadrature state
         * - Step 6: count consecutive completed steps in the raw direction, restarting the count
         *           whenever that direction changes; after four matching steps, establish it as
         *           the debounced direction
         * - Step 7: report the established debounced direction when one exists; otherwise report
         *           the raw direction immediately
         *
         * @param index Encoder index being filtered.
         * @param pair_state Current two-bit encoder pair state.
         * @param filtered_position Output storage for the filtered direction.
         * @param movement_elapsed_time Time since the previous movement, or `std::nullopt` when none exists.
         *
         * @return `true` when a complete filtered encoder step is available. Returns `false` while
         *         seeding the decoder, accumulating transitions, or processing no movement.
         */
        bool is_filtered(size_t                  index,
                         uint8_t                 pair_state,
                         Position&               filtered_position,
                         std::optional<uint32_t> movement_elapsed_time) override
        {
            auto position = Position::Stopped;
            pair_state &= ENCODER_STATE_MASK;
            bool process = true;

            if (!zlibs::utils::misc::bit_read(_encoder_data[index], ENCODER_DATA_VALID_BIT))
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

            if (position != Position::Stopped)
            {
                // Disable direction debouncing after an idle period without
                // discarding the quadrature state used to decode this step.
                if (!movement_elapsed_time.has_value() ||
                    (movement_elapsed_time.value() > ENCODERS_DEBOUNCE_RESET_TIME_MS))
                {
                    reset_direction_debounce(index);
                }

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

                _last_direction[index] = position;

                if (_debounce_direction[index] != Position::Stopped)
                {
                    filtered_position = _debounce_direction[index];
                }

                return true;
            }

            return false;
        }

        /**
         * @brief Resets all quadrature-decoder and direction-debounce state for one encoder.
         *
         * Unlike the idle-time direction reset, this full reset discards the previous A/B pair and
         * any partially accumulated step in addition to clearing direction history.
         *
         * @param index Encoder index to reset.
         */
        void reset(size_t index) override
        {
            reset_direction_debounce(index);
            _encoder_data[index]   = 0;
            _encoder_pulses[index] = 0;
        }

        private:
        static constexpr uint32_t ENCODERS_DEBOUNCE_RESET_TIME_MS = 50;
        static constexpr uint8_t  ENCODERS_DEBOUNCE_COUNT         = 4;
        static constexpr size_t   STORAGE_SIZE                    = Collection::size() ? Collection::size() : 1;
        static constexpr uint8_t  ENCODER_STATE_MASK              = 0x03;
        static constexpr uint8_t  ENCODER_DATA_VALID_BIT          = 7;
        static constexpr uint8_t  ENCODER_DATA_VALID_MASK         = 0x80;
        static constexpr uint8_t  ENCODER_LOOKUP_MASK             = 0x0F;
        static constexpr int8_t   ENCODER_LOOK_UP_TABLE[16]       = {
            0,
            1,
            -1,
            0,
            -1,
            0,
            0,
            1,
            1,
            0,
            0,
            -1,
            0,
            -1,
            1,
            0,
        };

        std::array<Position, STORAGE_SIZE> _last_direction     = {};
        std::array<Position, STORAGE_SIZE> _debounce_direction = {};
        std::array<uint8_t, STORAGE_SIZE>  _debounce_counter   = {};
        std::array<uint8_t, STORAGE_SIZE>  _encoder_data       = {};
        std::array<int8_t, STORAGE_SIZE>   _encoder_pulses     = {};

        /**
         * @brief Clears direction-debounce state without resetting quadrature decoding state.
         *
         * Preserves the previous A/B pair and accumulated pulses so movement can resume
         * after an idle period without discarding the first transition.
         *
         * @param index Encoder index to reset.
         */
        void reset_direction_debounce(size_t index)
        {
            _last_direction[index]     = Position::Stopped;
            _debounce_counter[index]   = 0;
            _debounce_direction[index] = Position::Stopped;
        }
    };
}    // namespace opendeck::firmware::io::encoders
