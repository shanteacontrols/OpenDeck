/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/io/digital/switches/shared/common.h"
#include "firmware/src/database/instance/impl/database.h"

#include <optional>

namespace opendeck::io::switches
{
    /**
     * @brief Database view used by the switch subsystem.
     */
    using Database = database::User<database::Config::Section::Switch,
                                    database::Config::Section::Encoder>;

    /**
     * @brief Hardware abstraction used by the switch subsystem.
     */
    class Hwa
    {
        public:
        virtual ~Hwa() = default;

        /**
         * @brief Initializes the switch backend.
         *
         * @return `true` if initialization succeeded, otherwise `false`.
         */
        virtual bool init() = 0;

        /**
         * @brief Returns the current state for one switch.
         *
         * @param index Switch index to query.
         *
         * @return Switch state, or `std::nullopt` when unavailable.
         */
        virtual std::optional<bool> state(size_t index) = 0;

        /**
         * @brief Returns the current encoded state for the encoder paired with a switch.
         *
         * @param index Switch index to query.
         *
         * @return Encoded encoder state, or `std::nullopt` when unavailable.
         */
        virtual std::optional<uint8_t> encoder_state(size_t index) = 0;

        /**
         * @brief Maps a switch index to the corresponding encoder index.
         *
         * @param index Switch index to map.
         *
         * @return Encoder index associated with the switch.
         */
        virtual size_t switch_to_encoder_index(size_t index) = 0;
    };

    /**
     * @brief Filter interface used to debounce and validate switch state changes.
     */
    class Filter
    {
        public:
        virtual ~Filter() = default;

        /**
         * @brief Filters one switch reading in place.
         *
         * @param index Switch index being processed.
         * @param state Input reading updated with the filtered state.
         *
         * @return `true` when the filtered state should be processed, otherwise `false`.
         */
        virtual bool is_filtered(size_t index, bool& state) = 0;
    };
}    // namespace opendeck::io::switches
