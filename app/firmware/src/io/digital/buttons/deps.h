/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "common.h"

#include "database/database.h"

#include <optional>

namespace io::buttons
{
    /**
     * @brief Database view used by the button subsystem.
     */
    using Database = database::User<database::Config::Section::Button,
                                    database::Config::Section::Encoder>;

    /**
     * @brief Hardware abstraction used by the button subsystem.
     */
    class Hwa
    {
        public:
        virtual ~Hwa() = default;

        /**
         * @brief Initializes the button backend.
         *
         * @return `true` if initialization succeeded, otherwise `false`.
         */
        virtual bool init() = 0;
        /**
         * @brief Returns the current state for one button.
         *
         * @param index Button index to query.
         *
         * @return Button state, or `std::nullopt` when unavailable.
         */
        virtual std::optional<bool> state(size_t index) = 0;
        /**
         * @brief Returns the current encoded state for the encoder paired with a button.
         *
         * @param index Button index to query.
         *
         * @return Encoded encoder state, or `std::nullopt` when unavailable.
         */
        virtual std::optional<uint8_t> encoder_state(size_t index) = 0;
        /**
         * @brief Maps a button index to the corresponding encoder index.
         *
         * @param index Button index to map.
         *
         * @return Encoder index associated with the button.
         */
        virtual size_t button_to_encoder_index(size_t index) = 0;
    };

    /**
     * @brief Filter interface used to debounce and validate button state changes.
     */
    class Filter
    {
        public:
        virtual ~Filter() = default;

        /**
         * @brief Filters one button reading in place.
         *
         * @param index Button index being processed.
         * @param state Input reading updated with the filtered state.
         *
         * @return `true` when the filtered state should be processed, otherwise `false`.
         */
        virtual bool is_filtered(size_t index, bool& state) = 0;
    };
}    // namespace io::buttons
