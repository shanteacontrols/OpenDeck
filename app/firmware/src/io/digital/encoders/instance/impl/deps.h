/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/io/digital/encoders/shared/common.h"
#include "firmware/src/database/instance/impl/database.h"
#include "firmware/src/io/digital/switches/shared/deps.h"

#include <optional>

namespace opendeck::firmware::io::encoders
{
    /**
     * @brief Database view used by the encoder subsystem.
     */
    using Database = database::User<database::Config::Section::Encoder,
                                    database::Config::Section::Global>;

    /**
     * @brief Hardware abstraction used by the encoder subsystem.
     */
    class Hwa
    {
        public:
        virtual ~Hwa() = default;

        /**
         * @brief Returns the current two-bit state of one encoder pair.
         *
         * @param index Encoder index to query.
         *
         * @return Encoded pair state, or `std::nullopt` when unavailable.
         */
        virtual std::optional<uint8_t> state(size_t index) = 0;
    };

    /**
     * @brief Filter interface used to debounce and shape encoder movement.
     */
    class Filter
    {
        public:
        static constexpr uint8_t PULSES_PER_STEP = 4;

        virtual ~Filter() = default;

        /**
         * @brief Decodes and filters one encoder hardware sample.
         *
         * @param index Encoder index being processed.
         * @param pair_state Current two-bit encoder pair state.
         * @param filtered_position Output position after filtering.
         * @param movement_elapsed_time Time since the previous movement, or `std::nullopt` when none exists.
         *
         * @return `true` when the filtered movement should be processed, otherwise `false`.
         */
        virtual bool is_filtered(size_t                  index,
                                 uint8_t                 pair_state,
                                 Position&               filtered_position,
                                 std::optional<uint32_t> movement_elapsed_time) = 0;

        /**
         * @brief Resets filter state for one encoder.
         *
         * @param index Encoder index to reset.
         */
        virtual void reset(size_t index) = 0;
    };
}    // namespace opendeck::firmware::io::encoders
