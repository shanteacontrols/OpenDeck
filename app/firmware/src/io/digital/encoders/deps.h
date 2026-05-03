/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "common.h"

#include "database/database.h"
#include "io/digital/buttons/deps.h"

#include <optional>

namespace opendeck::io::encoders
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
        virtual ~Filter() = default;

        /**
         * @brief Filters one encoder movement sample.
         *
         * @param index Encoder index being processed.
         * @param position Raw movement position decoded from the hardware state.
         * @param filtered_position Output position after filtering.
         * @param sample_taken_time Timestamp associated with the sample.
         *
         * @return `true` when the filtered movement should be processed, otherwise `false`.
         */
        virtual bool is_filtered(size_t    index,
                                 Position  position,
                                 Position& filtered_position,
                                 uint32_t  sample_taken_time) = 0;

        /**
         * @brief Resets filter state for one encoder.
         *
         * @param index Encoder index to reset.
         */
        virtual void reset(size_t index) = 0;

        /**
         * @brief Returns the last movement timestamp tracked by the filter.
         *
         * @param index Encoder index to query.
         *
         * @return Timestamp of the last accepted movement.
         */
        virtual uint32_t last_movement_time(size_t index) = 0;
    };
}    // namespace opendeck::io::encoders
