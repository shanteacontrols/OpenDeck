/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/io/i2c/shared/value_filter.h"

#include <inttypes.h>

namespace opendeck::firmware::io::i2c::sensor_apds9960
{
    /**
     * @brief Identifies configurable APDS9960 sensor settings stored in the database.
     */
    enum class Setting : uint8_t
    {
        ProximityGestureMode,
        EnableAmbientLight,
        EnableRgb,
        ProximityGain,
        AlsGain,
        Count
    };

    enum class ProximityGestureMode : uint8_t
    {
        Disabled,
        Proximity,
        Gesture,
        Count
    };

    using ProximityFilter    = ValueFilter<1>;
    using AmbientLightFilter = ValueFilter<1>;
    using RgbFilter          = ValueFilter<3>;
}    // namespace opendeck::firmware::io::i2c::sensor_apds9960
