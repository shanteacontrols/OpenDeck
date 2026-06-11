/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "firmware/src/io/i2c/peripherals/sensor_apds9960/instance/impl/mapper.h"

#include "zlibs/utils/misc/numeric.h"

using namespace opendeck::firmware::io::i2c::sensor_apds9960;
using namespace opendeck::firmware;

Mapper::Result Mapper::proximity_result(uint8_t value) const
{
    const auto info = read_database_info();

    return {
        .osc = {
            .payload = opendeck::firmware::signaling::OscSensorApds9960ProximitySignal{
                .value = proximity(value, info),
            },
            .direction = opendeck::firmware::signaling::SignalDirection::Out,
        },
    };
}

Mapper::Result Mapper::ambient_light_result(uint16_t value) const
{
    return {
        .osc = {
            .payload = opendeck::firmware::signaling::OscSensorApds9960AmbientLightSignal{
                .value = rgbc(value),
            },
            .direction = opendeck::firmware::signaling::SignalDirection::Out,
        },
    };
}

Mapper::Result Mapper::rgb_result(uint16_t clear, uint16_t red, uint16_t green, uint16_t blue) const
{
    return {
        .osc = {
            .payload = opendeck::firmware::signaling::OscSensorApds9960RgbSignal{
                .red   = rgb_channel(red, clear),
                .green = rgb_channel(green, clear),
                .blue  = rgb_channel(blue, clear),
            },
            .direction = opendeck::firmware::signaling::SignalDirection::Out,
        },
    };
}

Mapper::Result Mapper::gesture_result(opendeck::firmware::signaling::OscSensorApds9960Gesture gesture) const
{
    return {
        .osc = {
            .payload = opendeck::firmware::signaling::OscSensorApds9960GestureSignal{
                .gesture = gesture,
            },
            .direction = opendeck::firmware::signaling::SignalDirection::Out,
        },
    };
}

Mapper::DatabaseInfo Mapper::read_database_info() const
{
    return {
        .proximity_lower_value = static_cast<uint16_t>(_database.read(database::Config::Section::I2c::Apds9960, Setting::ProximityLowerValue)),
        .proximity_upper_value = static_cast<uint16_t>(_database.read(database::Config::Section::I2c::Apds9960, Setting::ProximityUpperValue)),
    };
}

uint8_t Mapper::proximity(uint8_t value, const DatabaseInfo& info) const
{
    return compute_value(value, info.proximity_lower_value, info.proximity_upper_value);
}

float Mapper::rgbc(uint16_t value) const
{
    const auto constrained = zlibs::utils::misc::constrain<uint16_t>(value, 0U, APDS9960_RGBC_MAX_COUNT);

    return static_cast<float>(constrained) / static_cast<float>(APDS9960_RGBC_MAX_COUNT);
}

float Mapper::rgb_channel(uint16_t value, uint16_t clear) const
{
    if (clear == 0)
    {
        return 0.0F;
    }

    return zlibs::utils::misc::constrain(static_cast<float>(value) / static_cast<float>(clear), 0.0F, 1.0F);
}

uint8_t Mapper::compute_value(uint16_t value, uint16_t lower, uint16_t upper) const
{
    if (lower == upper)
    {
        return value >= upper ? APDS9960_PROXIMITY_RAW_MAX : 0U;
    }

    uint32_t mapped = 0;

    if (lower > upper)
    {
        mapped = zlibs::utils::misc::map_range(static_cast<uint32_t>(value),
                                               static_cast<uint32_t>(upper),
                                               static_cast<uint32_t>(lower),
                                               static_cast<uint32_t>(0),
                                               static_cast<uint32_t>(APDS9960_PROXIMITY_RAW_MAX));
        mapped = APDS9960_PROXIMITY_RAW_MAX - mapped;
    }
    else
    {
        mapped = zlibs::utils::misc::map_range(static_cast<uint32_t>(value),
                                               static_cast<uint32_t>(lower),
                                               static_cast<uint32_t>(upper),
                                               static_cast<uint32_t>(0),
                                               static_cast<uint32_t>(APDS9960_PROXIMITY_RAW_MAX));
    }

    return static_cast<uint8_t>(zlibs::utils::misc::constrain(mapped,
                                                              static_cast<uint32_t>(0),
                                                              static_cast<uint32_t>(APDS9960_PROXIMITY_RAW_MAX)));
}
