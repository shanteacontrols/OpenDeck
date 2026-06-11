/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <inttypes.h>

namespace opendeck::firmware::io::i2c::sensor_vl53l4cx
{
    /** @brief Maximum VL53L4CX ranging distance in millimeters, per datasheet operating range. */
    constexpr inline uint16_t DISTANCE_MAX_MM = 6000;

    /**
     * @brief Identifies configurable VL53L4CX sensor settings stored in the database.
     */
    enum class Setting : uint8_t
    {
        EnableDistanceMm,
        EnableDistanceNorm,
        Smoothing,
        TrackingArea,
        DistanceMode,
        DistanceLowerValue,
        DistanceUpperValue,
        Count
    };

    /**
     * @brief Identifies the application-side distance smoothing profile.
     */
    enum class Smoothing : uint8_t
    {
        Off,
        Light,
        Medium,
        Heavy,
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
     * @brief Identifies the ST distance mode.
     */
    enum class DistanceMode : uint8_t
    {
        Medium,
        Long,
        Count
    };
}    // namespace opendeck::firmware::io::i2c::sensor_vl53l4cx
