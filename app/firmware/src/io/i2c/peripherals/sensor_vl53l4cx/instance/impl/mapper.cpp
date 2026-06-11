/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "firmware/src/io/i2c/peripherals/sensor_vl53l4cx/instance/impl/mapper.h"

#include "zlibs/utils/misc/numeric.h"

using namespace opendeck::firmware::io::i2c::sensor_vl53l4cx;
using namespace opendeck::firmware;

std::optional<Mapper::Result> Mapper::result(uint16_t distance_mm)
{
    const auto info                = read_database_info();
    const auto distance_normalized = compute_value(distance_mm, info.distance_lower_value, info.distance_upper_value);

    Result result = {};

    if (info.distance_mm_enabled &&
        (!_last_value.distance_mm_valid || (_last_value.distance_mm != distance_mm)))
    {
        result.distance_mm.emplace(distance_mm_signal(distance_mm));
        _last_value.distance_mm       = distance_mm;
        _last_value.distance_mm_valid = true;
    }

    if (info.distance_normalized_enabled &&
        (!_last_value.distance_normalized_valid || (_last_value.distance_normalized != distance_normalized)))
    {
        result.distance_normalized.emplace(distance_normalized_signal(distance_normalized));
        _last_value.distance_normalized       = distance_normalized;
        _last_value.distance_normalized_valid = true;
    }

    if (!result.distance_mm.has_value() && !result.distance_normalized.has_value())
    {
        return {};
    }

    return result;
}

std::optional<Mapper::Result> Mapper::last_result() const
{
    const auto info = read_database_info();

    Result result = {};

    if (info.distance_mm_enabled && _last_value.distance_mm_valid)
    {
        result.distance_mm.emplace(distance_mm_signal(_last_value.distance_mm));
    }

    if (info.distance_normalized_enabled && _last_value.distance_normalized_valid)
    {
        result.distance_normalized.emplace(distance_normalized_signal(_last_value.distance_normalized));
    }

    if (!result.distance_mm.has_value() && !result.distance_normalized.has_value())
    {
        return {};
    }

    return result;
}

void Mapper::reset()
{
    _last_value = {};
}

Mapper::DatabaseInfo Mapper::read_database_info() const
{
    return {
        .distance_mm_enabled         = _database.read(database::Config::Section::I2c::Vl53l4cx, Setting::EnableDistanceMm) != 0,
        .distance_normalized_enabled = _database.read(database::Config::Section::I2c::Vl53l4cx, Setting::EnableDistanceNorm) != 0,
        .distance_lower_value        = static_cast<uint16_t>(_database.read(database::Config::Section::I2c::Vl53l4cx, Setting::DistanceLowerValue)),
        .distance_upper_value        = static_cast<uint16_t>(_database.read(database::Config::Section::I2c::Vl53l4cx, Setting::DistanceUpperValue)),
    };
}

uint16_t Mapper::compute_value(uint16_t value, uint16_t lower, uint16_t upper) const
{
    if (lower == upper)
    {
        return value >= upper ? DISTANCE_MAX_MM : 0U;
    }

    uint32_t mapped = 0;

    if (lower > upper)
    {
        mapped = zlibs::utils::misc::map_range(static_cast<uint32_t>(value),
                                               static_cast<uint32_t>(upper),
                                               static_cast<uint32_t>(lower),
                                               static_cast<uint32_t>(0),
                                               static_cast<uint32_t>(DISTANCE_MAX_MM));
        mapped = DISTANCE_MAX_MM - mapped;
    }
    else
    {
        mapped = zlibs::utils::misc::map_range(static_cast<uint32_t>(value),
                                               static_cast<uint32_t>(lower),
                                               static_cast<uint32_t>(upper),
                                               static_cast<uint32_t>(0),
                                               static_cast<uint32_t>(DISTANCE_MAX_MM));
    }

    return static_cast<uint16_t>(zlibs::utils::misc::constrain(mapped,
                                                               static_cast<uint32_t>(0),
                                                               static_cast<uint32_t>(DISTANCE_MAX_MM)));
}

opendeck::firmware::signaling::OscSensorSignal Mapper::distance_mm_signal(uint16_t distance_mm) const
{
    return {
        .payload = opendeck::firmware::signaling::OscSensorVl53l4cxDistanceSignal{
            .value = distance_mm,
        },
        .direction = opendeck::firmware::signaling::SignalDirection::Out,
    };
}

opendeck::firmware::signaling::OscSensorSignal Mapper::distance_normalized_signal(uint16_t distance_normalized) const
{
    return {
        .payload = opendeck::firmware::signaling::OscSensorVl53l4cxDistanceNormSignal{
            .value = static_cast<float>(distance_normalized) / static_cast<float>(DISTANCE_MAX_MM),
        },
        .direction = opendeck::firmware::signaling::SignalDirection::Out,
    };
}
