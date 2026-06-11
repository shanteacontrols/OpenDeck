/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "firmware/src/io/i2c/peripherals/sensor_vl53l5cx/instance/impl/sensor_vl53l5cx.h"
#include "firmware/src/io/i2c/instance/impl/i2c.h"
#include "firmware/src/signaling/signaling.h"
#include "firmware/src/util/configurable/configurable.h"
#include "firmware/src/util/conversion/conversion.h"

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <algorithm>
#include <array>
#include <cstdio>
#include <limits>

using namespace opendeck::firmware::io::i2c::sensor_vl53l5cx;
using namespace opendeck::firmware::io::i2c;
using namespace opendeck::firmware;

namespace
{
    LOG_MODULE_REGISTER(sensor_vl53l5cx, CONFIG_OPENDECK_LOG_LEVEL);    // NOLINT

    constexpr int64_t  UPDATE_INTERVAL_MS            = 10;
    constexpr uint8_t  GRID_WIDTH_4X4                = 4;
    constexpr uint8_t  GRID_WIDTH_8X8                = 8;
    constexpr uint8_t  RANGING_FREQUENCY_4X4         = 60;
    constexpr uint8_t  RANGING_FREQUENCY_8X8         = 15;
    constexpr uint8_t  SMOOTHING_PERCENTAGE_OFF      = 100;
    constexpr uint8_t  SMOOTHING_PERCENTAGE_LIGHT    = 55;
    constexpr uint8_t  SMOOTHING_PERCENTAGE_MEDIUM   = 30;
    constexpr uint8_t  SMOOTHING_PERCENTAGE_HEAVY    = 15;
    constexpr uint8_t  INVALID_HOLD_FRAMES_OFF       = 0;
    constexpr uint8_t  INVALID_HOLD_FRAMES_LIGHT     = 2;
    constexpr uint8_t  INVALID_HOLD_FRAMES_MEDIUM    = 4;
    constexpr uint8_t  INVALID_HOLD_FRAMES_HEAVY     = 8;
    constexpr int64_t  OUTPUT_INTERVAL_LOW_MS        = 200;
    constexpr int64_t  OUTPUT_INTERVAL_NORMAL_MS     = 67;
    constexpr int64_t  OUTPUT_INTERVAL_HIGH_MS       = 17;
    constexpr uint8_t  TARGET_STATUS_VALID           = 5;
    constexpr uint8_t  TARGET_STATUS_SEMI_VALID      = 9;
    constexpr uint16_t PRESENCE_DISTANCE_SPAN_MIN_MM = 1;
    constexpr float    PRESENCE_ENERGY_SCALE         = 0.5F;
    constexpr uint32_t SMOOTHING_PERCENTAGE_DIVISOR  = 100;

    bool valid_target_status(uint8_t status)
    {
        return (status == TARGET_STATUS_VALID) || (status == TARGET_STATUS_SEMI_VALID);
    }

    size_t grid_width(Resolution resolution)
    {
        return resolution == Resolution::Zones4x4 ? GRID_WIDTH_4X4 : GRID_WIDTH_8X8;
    }

    uint8_t driver_resolution(Resolution resolution)
    {
        return resolution == Resolution::Zones4x4 ? VL53L5CX_RESOLUTION_4X4 : VL53L5CX_RESOLUTION_8X8;
    }

    uint8_t ranging_frequency(Resolution resolution)
    {
        return resolution == Resolution::Zones4x4 ? RANGING_FREQUENCY_4X4 : RANGING_FREQUENCY_8X8;
    }

    uint8_t smoothing_percentage(Smoothing smoothing)
    {
        switch (smoothing)
        {
        case Smoothing::Light:
            return SMOOTHING_PERCENTAGE_LIGHT;

        case Smoothing::Medium:
            return SMOOTHING_PERCENTAGE_MEDIUM;

        case Smoothing::Heavy:
            return SMOOTHING_PERCENTAGE_HEAVY;

        case Smoothing::Off:
        default:
            return SMOOTHING_PERCENTAGE_OFF;
        }
    }

    uint8_t invalid_hold_frames(Smoothing smoothing)
    {
        switch (smoothing)
        {
        case Smoothing::Light:
            return INVALID_HOLD_FRAMES_LIGHT;

        case Smoothing::Medium:
            return INVALID_HOLD_FRAMES_MEDIUM;

        case Smoothing::Heavy:
            return INVALID_HOLD_FRAMES_HEAVY;

        case Smoothing::Off:
        default:
            return INVALID_HOLD_FRAMES_OFF;
        }
    }

    int64_t output_interval_ms(OutputRate rate)
    {
        switch (rate)
        {
        case OutputRate::Low:
            return OUTPUT_INTERVAL_LOW_MS;

        case OutputRate::High:
            return OUTPUT_INTERVAL_HIGH_MS;

        case OutputRate::Normal:
        default:
            return OUTPUT_INTERVAL_NORMAL_MS;
        }
    }

}    // namespace

SensorVl53l5cx::SensorVl53l5cx(Hwa&      hwa,
                               Database& database)
    : _hwa(hwa)
    , _database(database)
    , _driver(&_hwa, -1, -1)
{
    _database.register_layout_init_provider(
        database::Config::Section::I2c::Vl53l5cx,
        [](size_t index) -> std::optional<uint32_t>
        {
            switch (static_cast<Setting>(index))
            {
            case Setting::OutputRate:
                return static_cast<uint32_t>(OutputRate::Normal);

            default:
                return {};
            }
        });

    ConfigHandler.register_config(
        sys::Config::Block::I2c,
        [this](uint8_t section, size_t index, uint16_t& value)
        {
            return sys_config_get(static_cast<sys::Config::Section::I2c>(section), index, value);
        },
        [this](uint8_t section, size_t index, uint16_t value)
        {
            return sys_config_set(static_cast<sys::Config::Section::I2c>(section), index, value);
        });

    I2c::register_peripheral(this);
}

bool SensorVl53l5cx::init(size_t address_index)
{
    if (_ranging)
    {
        _ranging = false;
        static_cast<void>(_driver.vl53l5cx_stop_ranging());
    }

    _initialized = false;

    if (address_index >= I2C_ADDRESSES.size())
    {
        return false;
    }

    _selected_i2c_address_index = address_index;

    if (!verify_identity())
    {
        return false;
    }

    static_cast<void>(_driver.begin());

    auto status = _driver.init_sensor(VL53L5CX_DEFAULT_I2C_ADDRESS);

    if (status != VL53L5CX_STATUS_OK)
    {
        LOG_DBG("VL53L5CX low-level init failed at 0x%02x: status=%d", i2c_address(), status);
        return false;
    }

    LOG_INF("VL53L5CX low-level init complete at 0x%02x", i2c_address());

    const auto configured_resolution = resolution();
    const auto configured_frequency  = ranging_frequency(configured_resolution);

    status = _driver.vl53l5cx_set_resolution(driver_resolution(configured_resolution));

    if (status != VL53L5CX_STATUS_OK)
    {
        LOG_DBG("VL53L5CX resolution setup failed at 0x%02x: status=%d", i2c_address(), status);
        return false;
    }

    status = _driver.vl53l5cx_set_target_order(VL53L5CX_TARGET_ORDER_CLOSEST);

    if (status != VL53L5CX_STATUS_OK)
    {
        LOG_DBG("VL53L5CX target order setup failed at 0x%02x: status=%d", i2c_address(), status);
        return false;
    }

    status = _driver.vl53l5cx_set_ranging_frequency_hz(configured_frequency);

    if (status != VL53L5CX_STATUS_OK)
    {
        LOG_DBG("VL53L5CX frequency setup failed at 0x%02x: status=%d", i2c_address(), status);
        return false;
    }

    status = _driver.vl53l5cx_start_ranging();

    if (status != VL53L5CX_STATUS_OK)
    {
        LOG_DBG("VL53L5CX ranging start failed at 0x%02x: status=%d", i2c_address(), status);
        return false;
    }

    LOG_INF("VL53L5CX ranging started at 0x%02x: resolution=%u frequency=%uHz",
            i2c_address(),
            driver_resolution(configured_resolution),
            configured_frequency);

    _initialized = true;
    _ranging     = true;
    reset_processing_state();

    return true;
}

bool SensorVl53l5cx::deinit()
{
    if (_ranging)
    {
        _ranging = false;
        static_cast<void>(_driver.vl53l5cx_stop_ranging());
    }

    static_cast<void>(_driver.end());
    _initialized = false;
    _ranging     = false;
    reset_processing_state();
    return true;
}

bool SensorVl53l5cx::update()
{
    if (!_initialized)
    {
        return false;
    }

    uint8_t ready  = 0;
    auto    status = _driver.vl53l5cx_check_data_ready(&ready);

    if (status != VL53L5CX_STATUS_OK)
    {
        LOG_DBG("VL53L5CX data-ready check failed at 0x%02x: status=%d", i2c_address(), status);
        _ranging = false;
        return false;
    }

    if (ready == 0U)
    {
        return true;
    }

    status = _driver.vl53l5cx_get_ranging_data(&_results);

    if (status != VL53L5CX_STATUS_OK)
    {
        LOG_DBG("VL53L5CX ranging data read failed at 0x%02x: status=%d", i2c_address(), status);
        _ranging = false;
        return false;
    }

    process_results();
    publish_results();

    return _initialized;
}

int64_t SensorVl53l5cx::update_interval_ms()
{
    return UPDATE_INTERVAL_MS;
}

std::span<const uint8_t> SensorVl53l5cx::i2c_addresses() const
{
    return I2C_ADDRESSES;
}

Resolution SensorVl53l5cx::resolution() const
{
    const auto value = _database.read(database::Config::Section::I2c::Vl53l5cx, Setting::Resolution);

    if (value >= static_cast<uint8_t>(Resolution::Count))
    {
        return Resolution::Zones8x8;
    }

    return static_cast<Resolution>(value);
}

Smoothing SensorVl53l5cx::smoothing() const
{
    const auto value = _database.read(database::Config::Section::I2c::Vl53l5cx, Setting::Smoothing);

    if (value >= static_cast<uint8_t>(Smoothing::Count))
    {
        return Smoothing::Off;
    }

    return static_cast<Smoothing>(value);
}

OutputMode SensorVl53l5cx::output_mode() const
{
    const auto value = _database.read(database::Config::Section::I2c::Vl53l5cx, Setting::OutputMode);

    if (value >= static_cast<uint8_t>(OutputMode::Count))
    {
        return OutputMode::Disabled;
    }

    return static_cast<OutputMode>(value);
}

uint16_t SensorVl53l5cx::distance_lower_value() const
{
    return static_cast<uint16_t>(std::min<uint32_t>(
        _database.read(database::Config::Section::I2c::Vl53l5cx, Setting::DistanceLowerValue),
        DISTANCE_MAX_MM));
}

uint16_t SensorVl53l5cx::distance_upper_value() const
{
    const auto value = _database.read(database::Config::Section::I2c::Vl53l5cx, Setting::DistanceUpperValue);

    return value == 0 ? DISTANCE_MAX_MM : static_cast<uint16_t>(std::min<uint32_t>(value, DISTANCE_MAX_MM));
}

bool SensorVl53l5cx::invert_x() const
{
    return _database.read(database::Config::Section::I2c::Vl53l5cx, Setting::InvertX) != 0;
}

bool SensorVl53l5cx::invert_y() const
{
    return _database.read(database::Config::Section::I2c::Vl53l5cx, Setting::InvertY) != 0;
}

Rotation SensorVl53l5cx::rotation() const
{
    const auto value = _database.read(database::Config::Section::I2c::Vl53l5cx, Setting::Rotation);

    if (value >= static_cast<uint8_t>(Rotation::Count))
    {
        return Rotation::Deg0;
    }

    return static_cast<Rotation>(value);
}

OutputRate SensorVl53l5cx::output_rate() const
{
    const auto value = _database.read(database::Config::Section::I2c::Vl53l5cx, Setting::OutputRate);

    if (value >= static_cast<uint8_t>(OutputRate::Count))
    {
        return OutputRate::Normal;
    }

    return static_cast<OutputRate>(value);
}

void SensorVl53l5cx::reset_processing_state()
{
    _grid                  = {};
    _smoothed_distance_mm  = {};
    _invalid_frame_count   = {};
    _has_smoothed_distance = {};
    _last_publish_ms       = 0;
}

void SensorVl53l5cx::process_results()
{
    _grid = {};

    const auto width          = grid_width(resolution());
    const auto configured_min = distance_lower_value();
    const auto configured_max = distance_upper_value();
    const auto hold_frames    = invalid_hold_frames(smoothing());
    size_t     valid_zones    = 0;
    int32_t    min_mm         = std::numeric_limits<int32_t>::max();
    int32_t    max_mm         = 0;

    for (size_t y = 0; y < width; y++)
    {
        for (size_t x = 0; x < width; x++)
        {
            const auto source_zone = x + (y * width);
            const auto target      = oriented_point(x, y, width);
            auto&      zone        = _grid[target.x + (target.y * width)];
            const bool raw_valid   = (_results.nb_target_detected[source_zone] != 0U) &&
                                     valid_target_status(_results.target_status[source_zone]) &&
                                     (_results.distance_mm[source_zone] > 0) &&
                                     (_results.distance_mm[source_zone] >= configured_min) &&
                                     (_results.distance_mm[source_zone] <= configured_max);

            if (raw_valid)
            {
                const auto filtered = smooth_distance(source_zone, static_cast<uint16_t>(_results.distance_mm[source_zone]));

                _invalid_frame_count[source_zone] = 0;
                zone.distance_mm                  = filtered;
                zone.valid                        = true;
            }
            else if (_has_smoothed_distance[source_zone] &&
                     (_invalid_frame_count[source_zone] < hold_frames))
            {
                _invalid_frame_count[source_zone]++;
                zone.distance_mm = _smoothed_distance_mm[source_zone];
                zone.valid       = true;
            }
            else
            {
                _has_smoothed_distance[source_zone] = false;
                _invalid_frame_count[source_zone]   = 0;
            }

            if (zone.valid)
            {
                valid_zones++;
                min_mm = std::min(min_mm, zone.distance_mm);
                max_mm = std::max(max_mm, zone.distance_mm);
            }
        }
    }

    LOG_DBG("VL53L5CX frame: valid=%u min=%dmm max=%dmm",
            static_cast<unsigned>(valid_zones),
            valid_zones == 0U ? 0 : min_mm,
            valid_zones == 0U ? 0 : max_mm);
}

void SensorVl53l5cx::publish_results()
{
    const auto now = k_uptime_get();

    if ((now - _last_publish_ms) < output_interval_ms(output_rate()))
    {
        return;
    }

    _last_publish_ms = now;

    const auto width = grid_width(resolution());

    switch (output_mode())
    {
    case OutputMode::Disabled:
        break;

    case OutputMode::Nearest:
        publish_nearest(width);
        break;

    case OutputMode::Centroid:
        publish_centroid(width);
        break;

    case OutputMode::Presence:
        publish_presence(width);
        break;

    default:
        publish_grid(width);
        break;
    }
}

void SensorVl53l5cx::publish_grid(size_t width) const
{
    for (size_t row = 0; row < width; row++)
    {
        signaling::OscSensorVl53l5cxRowSignal payload = {
            .row         = row,
            .width       = width,
            .distance_mm = {},
        };

        for (size_t x = 0; x < width; x++)
        {
            const auto& zone       = _grid[x + (row * width)];
            payload.distance_mm[x] = zone.valid ? zone.distance_mm : 0;
        }

        signaling::publish(signaling::OscSensorSignal{
            .payload = payload,
        });
    }
}

void SensorVl53l5cx::publish_nearest(size_t width) const
{
    signaling::OscSensorVl53l5cxNearestSignal payload = {};
    int32_t                                   min_mm  = std::numeric_limits<int32_t>::max();

    for (size_t y = 0; y < width; y++)
    {
        for (size_t x = 0; x < width; x++)
        {
            const auto  zone_index = x + (y * width);
            const auto& zone       = _grid[zone_index];

            if (!zone.valid || (zone.distance_mm >= min_mm))
            {
                continue;
            }

            min_mm              = zone.distance_mm;
            payload.distance_mm = zone.distance_mm;
            payload.zone        = static_cast<int32_t>(zone_index);
            payload.x           = static_cast<int32_t>(x);
            payload.y           = static_cast<int32_t>(y);
        }
    }

    if (min_mm == std::numeric_limits<int32_t>::max())
    {
        payload.zone = -1;
        payload.x    = -1;
        payload.y    = -1;
    }

    signaling::publish(signaling::OscSensorSignal{
        .payload = payload,
    });
}

void SensorVl53l5cx::publish_centroid(size_t width) const
{
    float  x_sum        = 0.0F;
    float  y_sum        = 0.0F;
    float  distance_sum = 0.0F;
    size_t active_count = 0;

    for (size_t y = 0; y < width; y++)
    {
        for (size_t x = 0; x < width; x++)
        {
            const auto& zone = _grid[x + (y * width)];

            if (!zone.valid)
            {
                continue;
            }

            x_sum += static_cast<float>(x);
            y_sum += static_cast<float>(y);
            distance_sum += static_cast<float>(zone.distance_mm);
            active_count++;
        }
    }

    signaling::OscSensorVl53l5cxCentroidSignal payload = {
        .x            = active_count == 0 ? -1.0F : x_sum / static_cast<float>(active_count),
        .y            = active_count == 0 ? -1.0F : y_sum / static_cast<float>(active_count),
        .distance_mm  = active_count == 0 ? 0.0F : distance_sum / static_cast<float>(active_count),
        .active_zones = static_cast<int32_t>(active_count),
    };

    signaling::publish(signaling::OscSensorSignal{
        .payload = payload,
    });
}

void SensorVl53l5cx::publish_presence(size_t width) const
{
    size_t  active_count = 0;
    int32_t nearest_mm   = std::numeric_limits<int32_t>::max();

    for (size_t zone_index = 0; zone_index < (width * width); zone_index++)
    {
        const auto& zone = _grid[zone_index];

        if (!zone.valid)
        {
            continue;
        }

        active_count++;
        nearest_mm = std::min(nearest_mm, zone.distance_mm);
    }

    const auto lower       = distance_lower_value();
    const auto upper       = std::max<uint16_t>(distance_upper_value(), lower + PRESENCE_DISTANCE_SPAN_MIN_MM);
    const auto nearest     = nearest_mm == std::numeric_limits<int32_t>::max() ? 0 : nearest_mm;
    const auto active_part = static_cast<float>(active_count) / static_cast<float>(width * width);
    const auto near_part   = nearest == 0 ? 0.0F
                                          : 1.0F - std::clamp((static_cast<float>(nearest) - static_cast<float>(lower)) /
                                                                  (static_cast<float>(upper) - static_cast<float>(lower)),
                                                              0.0F,
                                                              1.0F);

    signaling::OscSensorVl53l5cxPresenceSignal payload = {
        .present      = active_count != 0 ? 1 : 0,
        .active_zones = static_cast<int32_t>(active_count),
        .nearest_mm   = nearest,
        .energy       = std::clamp((active_part + near_part) * PRESENCE_ENERGY_SCALE, 0.0F, 1.0F),
    };

    signaling::publish(signaling::OscSensorSignal{
        .payload = payload,
    });
}

SensorVl53l5cx::Point SensorVl53l5cx::oriented_point(size_t x, size_t y, size_t width) const
{
    Point point = {};

    switch (rotation())
    {
    case Rotation::Deg90:
        point = {
            .x = width - 1U - y,
            .y = x,
        };
        break;

    case Rotation::Deg180:
        point = {
            .x = width - 1U - x,
            .y = width - 1U - y,
        };
        break;

    case Rotation::Deg270:
        point = {
            .x = y,
            .y = width - 1U - x,
        };
        break;

    case Rotation::Deg0:
    default:
        point = {
            .x = x,
            .y = y,
        };
        break;
    }

    if (invert_x())
    {
        point.x = width - 1U - point.x;
    }

    if (invert_y())
    {
        point.y = width - 1U - point.y;
    }

    return point;
}

uint16_t SensorVl53l5cx::smooth_distance(size_t zone, uint16_t distance_mm)
{
    const auto percentage = smoothing_percentage(smoothing());

    if (!_has_smoothed_distance[zone] || (percentage == SMOOTHING_PERCENTAGE_OFF))
    {
        _smoothed_distance_mm[zone]  = distance_mm;
        _has_smoothed_distance[zone] = true;

        return distance_mm;
    }

    const uint32_t filtered = (static_cast<uint32_t>(percentage) * distance_mm) +
                              ((SMOOTHING_PERCENTAGE_DIVISOR - static_cast<uint32_t>(percentage)) *
                               _smoothed_distance_mm[zone]);

    _smoothed_distance_mm[zone] = static_cast<uint16_t>(filtered / SMOOTHING_PERCENTAGE_DIVISOR);

    return _smoothed_distance_mm[zone];
}

uint8_t SensorVl53l5cx::i2c_address() const
{
    return I2C_ADDRESSES[_selected_i2c_address_index];
}

std::optional<uint8_t> SensorVl53l5cx::read_register(uint16_t reg)
{
    const std::array<uint8_t, 2> write_buffer = {
        static_cast<uint8_t>(reg >> 8U),
        static_cast<uint8_t>(reg),
    };

    std::array<uint8_t, 1> read_buffer = {};

    if (!_hwa.write_read(i2c_address(), write_buffer, read_buffer))
    {
        return {};
    }

    return read_buffer[0];
}

bool SensorVl53l5cx::write_register(uint16_t reg, uint8_t value)
{
    const std::array<uint8_t, 3> buffer = {
        static_cast<uint8_t>(reg >> 8U),
        static_cast<uint8_t>(reg),
        value,
    };

    return _hwa.write(i2c_address(), buffer);
}

bool SensorVl53l5cx::select_page(uint8_t page)
{
    return write_register(VL53L5CX_REGISTER_PAGE_SELECT, page);
}

bool SensorVl53l5cx::verify_identity()
{
    if (!select_page(VL53L5CX_IDENTITY_PAGE))
    {
        LOG_DBG("VL53L5CX page select failed at 0x%02x", i2c_address());
        return false;
    }

    const auto device_id   = read_register(VL53L5CX_REGISTER_DEVICE_ID);
    const auto revision_id = read_register(VL53L5CX_REGISTER_REVISION_ID);
    const bool restored    = select_page(VL53L5CX_DEFAULT_PAGE);

    if (!restored)
    {
        LOG_DBG("VL53L5CX page restore failed at 0x%02x", i2c_address());
        return false;
    }

    if (!device_id.has_value() || !revision_id.has_value())
    {
        LOG_DBG("VL53L5CX responded at 0x%02x, but identity read failed", i2c_address());
        return false;
    }

    if ((device_id.value() != VL53L5CX_EXPECTED_DEVICE_ID) ||
        (revision_id.value() != VL53L5CX_EXPECTED_REVISION_ID))
    {
        LOG_DBG("VL53L5CX candidate at 0x%02x has unexpected identity 0x%02x/0x%02x",
                i2c_address(),
                device_id.value(),
                revision_id.value());
        return false;
    }

    return true;
}

std::optional<uint8_t> SensorVl53l5cx::sys_config_get(sys::Config::Section::I2c section, size_t index, uint16_t& value)
{
    if (section != sys::Config::Section::I2c::Vl53l5cx)
    {
        return {};
    }

    uint32_t read_value = 0;

    auto result = _database.read(util::Conversion::sys_2_db_section(section), index, read_value)
                      ? sys::Config::Status::Ack
                      : sys::Config::Status::ErrorRead;

    value = read_value;

    return result;
}

std::optional<uint8_t> SensorVl53l5cx::sys_config_set(sys::Config::Section::I2c section, size_t index, uint16_t value)
{
    if (section != sys::Config::Section::I2c::Vl53l5cx)
    {
        return {};
    }

    const auto setting = static_cast<Setting>(index);

    switch (setting)
    {
    case Setting::Resolution:
    {
        if (value >= static_cast<uint8_t>(Resolution::Count))
        {
            return sys::Config::Status::ErrorWrite;
        }
    }
    break;

    case Setting::Smoothing:
    {
        if (value >= static_cast<uint8_t>(Smoothing::Count))
        {
            return sys::Config::Status::ErrorWrite;
        }
    }
    break;

    case Setting::OutputMode:
    {
        if (value >= static_cast<uint8_t>(OutputMode::Count))
        {
            return sys::Config::Status::ErrorWrite;
        }
    }
    break;

    case Setting::DistanceLowerValue:
    case Setting::DistanceUpperValue:
    {
        if (value > DISTANCE_MAX_MM)
        {
            return sys::Config::Status::ErrorWrite;
        }
    }
    break;

    case Setting::InvertX:
    case Setting::InvertY:
    {
        if (value > 1)
        {
            return sys::Config::Status::ErrorWrite;
        }
    }
    break;

    case Setting::Rotation:
    {
        if (value >= static_cast<uint8_t>(Rotation::Count))
        {
            return sys::Config::Status::ErrorWrite;
        }
    }
    break;

    case Setting::OutputRate:
    {
        if (value >= static_cast<uint8_t>(OutputRate::Count))
        {
            return sys::Config::Status::ErrorWrite;
        }
    }
    break;

    default:
        return sys::Config::Status::ErrorWrite;
    }

    auto result = _database.update(util::Conversion::sys_2_db_section(section), index, value)
                      ? sys::Config::Status::Ack
                      : sys::Config::Status::ErrorWrite;

    if ((result == sys::Config::Status::Ack) &&
        (setting == Setting::Resolution))
    {
        result = init(_selected_i2c_address_index)
                     ? sys::Config::Status::Ack
                     : sys::Config::Status::ErrorWrite;
    }
    else if (result == sys::Config::Status::Ack)
    {
        reset_processing_state();
    }

    return result;
}
