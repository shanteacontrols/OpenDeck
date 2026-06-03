/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <inttypes.h>

namespace opendeck::firmware::io::i2c::sensor_cap1188
{
    /**
     * @brief Identifies configurable CAP1188 sensor settings stored in the database.
     */
    enum class Setting : uint8_t
    {
        Sensitivity,
        Count
    };

    /**
     * @brief Identifies the CAP1188 touch sensitivity preset.
     */
    enum class Sensitivity : uint8_t
    {
        Low,
        Medium,
        High,
        Count
    };
}    // namespace opendeck::firmware::io::i2c::sensor_cap1188
