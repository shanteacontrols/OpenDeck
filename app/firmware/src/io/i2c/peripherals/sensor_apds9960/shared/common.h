/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <inttypes.h>

namespace opendeck::firmware::io::i2c::sensor_apds9960
{
    /**
     * @brief Identifies configurable APDS9960 sensor settings stored in the database.
     */
    enum class Setting : uint8_t
    {
        EnableProximity,
        EnableAmbientLight,
        EnableRgb,
        EnableGesture,
        ProximityGain,
        AlsGain,
        Count
    };

}    // namespace opendeck::firmware::io::i2c::sensor_apds9960
