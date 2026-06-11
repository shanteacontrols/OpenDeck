/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <inttypes.h>

namespace opendeck::firmware::io::i2c::sensor_vl53l5cx
{
    /** @brief Maximum VL53L5CX ranging distance in millimeters, per datasheet operating range. */
    constexpr inline uint16_t DISTANCE_MAX_MM = 4000;

    /**
     * @brief Identifies configurable VL53L5CX sensor settings stored in the database.
     */
    enum class Setting : uint8_t
    {
        Resolution,
        Smoothing,
        OutputMode,
        DistanceLowerValue,
        DistanceUpperValue,
        InvertX,
        InvertY,
        Rotation,
        OutputRate,
        Count
    };

    /**
     * @brief Identifies the active ranging grid resolution.
     */
    enum class Resolution : uint8_t
    {
        Zones8x8,
        Zones4x4,
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
     * @brief Identifies the OSC data shape published by the sensor.
     */
    enum class OutputMode : uint8_t
    {
        Disabled,
        Grid,
        Nearest,
        Centroid,
        Presence,
        Count
    };

    /**
     * @brief Identifies clockwise grid rotation before OSC output.
     */
    enum class Rotation : uint8_t
    {
        Deg0,
        Deg90,
        Deg180,
        Deg270,
        Count
    };

    /**
     * @brief Identifies the maximum OSC publish rate.
     */
    enum class OutputRate : uint8_t
    {
        Low,
        Normal,
        High,
        Count
    };
}    // namespace opendeck::firmware::io::i2c::sensor_vl53l5cx
