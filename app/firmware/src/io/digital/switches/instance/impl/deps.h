/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/io/digital/switches/shared/deps.h"
#include "firmware/src/database/instance/impl/database.h"

namespace opendeck::firmware::io::switches
{
    /**
     * @brief Database view used by the switch subsystem.
     */
    using Database = database::User<database::Config::Section::Switch,
                                    database::Config::Section::Encoder>;

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
}    // namespace opendeck::firmware::io::switches
