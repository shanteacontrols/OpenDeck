/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/io/i2c/shared/value_filter.h"

#include <inttypes.h>

namespace opendeck::firmware::io::i2c::sensor_apds9960
{
    /** @brief Maximum 8-bit APDS9960 proximity data value. */
    constexpr inline uint16_t APDS9960_PROXIMITY_RAW_MAX = UINT8_MAX;

    /** @brief Maximum APDS9960 RGBC count for the firmware's configured 10-cycle integration time. */
    constexpr inline uint16_t APDS9960_RGBC_MAX_COUNT = 10241;

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
        ProximityLowerValue,
        ProximityUpperValue,
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
