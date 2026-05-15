/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/io/digital/drivers/driver_base.h"
#include "firmware/src/io/digital/remap.h"

#include <optional>

namespace opendeck::io::digital
{
    /**
     * @brief Shared cache of the most recent digital scan frame.
     */
    class FrameStore
    {
        public:
        /**
         * @brief Constructs the frame store around the active digital driver.
         *
         * @param driver Driver used for switch/encoder index mapping.
         */
        explicit FrameStore(drivers::DriverBase& driver)
            : _driver(driver)
        {}

        /**
         * @brief Stores a new digital frame and marks it valid.
         *
         * @param frame Frame to cache.
         */
        void set_frame(const drivers::Frame& frame)
        {
            _frame       = frame;
            _frame_valid = true;
        }

        /**
         * @brief Invalidates the cached frame.
         */
        void clear()
        {
            _frame_valid = false;
        }

        /**
         * @brief Returns the cached state of one digital component.
         *
         * @param index Digital component index to query.
         *
         * @return Cached state, or `std::nullopt` when the frame is invalid or the index is out of range.
         */
        std::optional<bool> state(size_t index) const
        {
            const size_t physical_index = Remap::physical(index);

            if (!_frame_valid || (physical_index >= _frame.size()))
            {
                return {};
            }

            return _frame[physical_index];
        }

        /**
         * @brief Returns the cached two-bit state for one encoder.
         *
         * @param index Encoder index to query.
         *
         * @return Encoded pair state, or `std::nullopt` when unavailable.
         */
        std::optional<uint8_t> encoder_state(size_t index) const
        {
            static constexpr uint8_t ENCODER_A_MASK = 0x02;
            static constexpr uint8_t ENCODER_B_MASK = 0x01;

            const size_t encoder_count = _driver.encoder_count();

            if (!_frame_valid || (encoder_count == 0) || (index >= encoder_count))
            {
                return {};
            }

            const size_t component_a = Remap::physical(_driver.encoder_component_from_encoder(index, drivers::EncoderComponent::A));
            const size_t component_b = Remap::physical(_driver.encoder_component_from_encoder(index, drivers::EncoderComponent::B));

            if ((component_a >= _frame.size()) || (component_b >= _frame.size()))
            {
                return {};
            }

            return static_cast<uint8_t>((_frame[component_a] ? ENCODER_A_MASK : 0x00U) |
                                        (_frame[component_b] ? ENCODER_B_MASK : 0x00U));
        }

        /**
         * @brief Maps a switch index to the corresponding encoder index.
         *
         * @param index Switch index to map.
         *
         * @return Encoder index associated with the switch.
         */
        size_t switch_to_encoder_index(size_t index) const
        {
            return _driver.switch_to_encoder_index(index);
        }

        private:
        drivers::DriverBase& _driver;
        drivers::Frame       _frame       = {};
        bool                 _frame_valid = false;
    };
}    // namespace opendeck::io::digital
