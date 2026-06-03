/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/io/shared/common.h"

namespace opendeck::firmware::io::touchscreen
{
    /**
     * @brief Flattened collection of touchscreen components.
     */
    class Collection : public io::common::BaseCollection<CONFIG_PROJECT_TARGET_TOUCHSCREEN_COMPONENT_COUNT>
    {
        public:
        /**
         * @brief Prevents instantiation of this compile-time touchscreen collection descriptor.
         */
        Collection() = delete;
    };

    /**
     * @brief Touchscreen setting parameters stored in the database.
     */
    enum class Setting : uint8_t
    {
        Enable,
        Model,
        Brightness,
        InitialScreen,
        Reserved,
        Count
    };
}    // namespace opendeck::firmware::io::touchscreen
