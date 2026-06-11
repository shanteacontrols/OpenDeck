/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <inttypes.h>

namespace opendeck::firmware::io::i2c::sensor_bno085
{
    /**
     * @brief Identifies configurable BNO085 sensor settings stored in the database.
     */
    enum class Setting : uint8_t
    {
        EnableQuaternion,
        EnableEuler,
        EnableGyroscope,
        EnableLinearAcceleration,
        EnableGravity,
        Smoothing,
        Count
    };

    /**
     * @brief Identifies the application-side IMU output smoothing profile.
     */
    enum class Smoothing : uint8_t
    {
        Off,
        Light,
        Medium,
        Heavy,
        Count
    };
}    // namespace opendeck::firmware::io::i2c::sensor_bno085
