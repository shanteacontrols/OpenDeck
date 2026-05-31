/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <array>
#include <inttypes.h>

namespace opendeck::io::i2c::sensor_vl53l4cx
{
    /**
     * @brief Identifies configurable VL53L4CX sensor settings stored in the database.
     */
    enum class Setting : uint8_t
    {
        EnableDistance,
        TrackingArea,
        Response,
        DistanceMode,
        Count
    };

    /**
     * @brief Identifies the active receiver area used for ranging.
     */
    enum class TrackingArea : uint8_t
    {
        Narrow,
        Medium,
        Wide,
        Full,
        Count
    };

    /**
     * @brief Identifies the measurement timing profile.
     */
    enum class Response : uint8_t
    {
        Fast,
        Balanced,
        Stable,
        Count
    };

    /**
     * @brief Identifies the ST distance mode.
     */
    enum class DistanceMode : uint8_t
    {
        Medium,
        Long,
        Count
    };

    constexpr inline std::array<uint8_t, 1> I2C_ADDRESSES = {
        0x29,
    };

    constexpr inline uint16_t VL53L4CX_REGISTER_SOFT_RESET = 0x0000;
}    // namespace opendeck::io::i2c::sensor_vl53l4cx
