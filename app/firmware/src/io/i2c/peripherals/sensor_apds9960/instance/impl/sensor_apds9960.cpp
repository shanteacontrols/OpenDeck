/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "firmware/src/io/i2c/peripherals/sensor_apds9960/instance/impl/sensor_apds9960.h"
#include "firmware/src/io/i2c/instance/impl/i2c.h"
#include "firmware/src/io/shared/common.h"
#include "firmware/src/util/configurable/configurable.h"
#include "firmware/src/util/conversion/conversion.h"

#include "zlibs/utils/misc/bit.h"

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/byteorder.h>

#include <algorithm>
#include <cstdlib>

using namespace opendeck::firmware::io::i2c::sensor_apds9960;
using namespace opendeck::firmware;

namespace zmisc = zlibs::utils::misc;

namespace
{
    LOG_MODULE_REGISTER(sensor_apds9960, CONFIG_OPENDECK_LOG_LEVEL);    // NOLINT

    const char* gesture_to_string(signaling::OscSensorApds9960Gesture gesture)
    {
        switch (gesture)
        {
        case signaling::OscSensorApds9960Gesture::Up:
            return "up";

        case signaling::OscSensorApds9960Gesture::Down:
            return "down";

        case signaling::OscSensorApds9960Gesture::Left:
            return "left";

        case signaling::OscSensorApds9960Gesture::Right:
            return "right";

        case signaling::OscSensorApds9960Gesture::None:
        default:
            return "none";
        }
    }
}    // namespace

SensorApds9960::SensorApds9960(Hwa&      hwa,
                               Database& database)
    : _hwa(hwa)
    , _database(database)
    , _mapper(database)
{
    _database.register_layout_init_provider(
        database::Config::Section::I2c::Apds9960,
        [](size_t index) -> std::optional<uint32_t>
        {
            switch (static_cast<Setting>(index))
            {
            case Setting::ProximityGain:
                return 2;

            case Setting::AlsGain:
                return 1;

            case Setting::ProximityUpperValue:
                return APDS9960_PROXIMITY_RAW_MAX;

            default:
                return {};
            }
        });

    ConfigHandler.register_config(
        sys::Config::Block::I2c,
        // read
        [this](uint8_t section, size_t index, uint16_t& value)
        {
            return sys_config_get(static_cast<sys::Config::Section::I2c>(section), index, value);
        },

        // write
        [this](uint8_t section, size_t index, uint16_t value)
        {
            return sys_config_set(static_cast<sys::Config::Section::I2c>(section), index, value);
        });

    signaling::subscribe<signaling::ForcedRefreshStart>(
        [this](const signaling::ForcedRefreshStart&)
        {
            publish_value_states();
        });

    I2c::register_peripheral(this);
}

bool SensorApds9960::init(size_t address_index)
{
    _found       = false;
    _initialized = false;

    if (address_index >= I2C_ADDRESSES.size())
    {
        return false;
    }

    _selected_i2c_address_index = address_index;

    const auto id = read_register(APDS9960_REGISTER_ID);

    if (!id.has_value())
    {
        LOG_DBG("APDS9960 responded at 0x%02x, but ID read failed", i2c_address());
        return false;
    }

    if (std::find(APDS9960_DEVICE_IDS.begin(), APDS9960_DEVICE_IDS.end(), id.value()) == APDS9960_DEVICE_IDS.end())
    {
        LOG_DBG("APDS9960 candidate at 0x%02x has unexpected ID 0x%02x", i2c_address(), id.value());
        return false;
    }

    _found = true;

    if (!configure_sensor())
    {
        LOG_DBG("APDS9960 setup failed");
        return false;
    }

    _initialized        = true;
    _read_failure_count = 0;

    return _initialized;
}

bool SensorApds9960::update()
{
    if (!_initialized)
    {
        return false;
    }

    uint8_t status = 0;

    if ((proximity_gesture_mode() == ProximityGestureMode::Proximity) ||
        output_enabled(Setting::EnableRgb) ||
        output_enabled(Setting::EnableAmbientLight))
    {
        const auto read_status = read_register(APDS9960_REGISTER_STATUS);

        if (!read_status.has_value())
        {
            return recoverable_i2c_read_failure();
        }

        status = read_status.value();
    }

    return read_proximity(status) &&
           read_gesture() &&
           read_ambient_light_and_rgb(status);
}

constexpr std::string_view SensorApds9960::name() const
{
    return "sensor_apds9960";
}

int64_t SensorApds9960::update_interval_ms()
{
    return APDS9960_UPDATE_INTERVAL_MS;
}

std::span<const uint8_t> SensorApds9960::i2c_addresses() const
{
    return I2C_ADDRESSES;
}

bool SensorApds9960::read_proximity(uint8_t status)
{
    if (proximity_gesture_mode() != ProximityGestureMode::Proximity)
    {
        return true;
    }

    if ((status & APDS9960_STATUS_PVALID) == 0)
    {
        return true;
    }

    const auto proximity = read_register(APDS9960_REGISTER_PDATA);

    if (!proximity.has_value())
    {
        return recoverable_i2c_read_failure();
    }

    uint8_t value = proximity.value();

    if (_has_proximity_value)
    {
        value = _proximity_filter.value(value);
    }

    if (_has_proximity_value && (_last_proximity_value == value))
    {
        return true;
    }

    if (!_has_proximity_value)
    {
        _proximity_filter.reset(value);
    }

    _has_proximity_value  = true;
    _last_proximity_value = value;

    publish_result(_mapper.proximity_result(value));

    return true;
}

bool SensorApds9960::read_gesture()
{
    if (proximity_gesture_mode() != ProximityGestureMode::Gesture)
    {
        return true;
    }

    auto gesture = decode_gesture_fifo();

    if (!gesture.has_value())
    {
        return recoverable_i2c_read_failure();
    }

    const int64_t now_ms = k_uptime_get();

    if ((_last_gesture_send_ms != 0) &&
        ((now_ms - _last_gesture_send_ms) < GESTURE_SEND_COOLDOWN_MS))
    {
        LOG_DBG("APDS9960 gesture skipped by cooldown: %s", gesture_to_string(gesture.value()));
        return true;
    }

    publish_result(_mapper.gesture_result(output_gesture(gesture.value())));

    _last_gesture_send_ms = now_ms;

    return true;
}

bool SensorApds9960::read_ambient_light_and_rgb(uint8_t status)
{
    const bool rgb_enabled           = output_enabled(Setting::EnableRgb);
    const bool ambient_light_enabled = output_enabled(Setting::EnableAmbientLight);

    if (!rgb_enabled && !ambient_light_enabled)
    {
        return true;
    }

    if ((status & APDS9960_STATUS_AVALID) == 0)
    {
        return true;
    }

    std::array<uint8_t, APDS9960_LIGHT_DATA_SIZE> light = {};

    if (!read_register_block(APDS9960_REGISTER_CDATAL, std::span<uint8_t>(light)))
    {
        return recoverable_i2c_read_failure();
    }

    const auto ambient_light = sys_get_le16(light.data());
    const auto red           = sys_get_le16(light.data() + 2);
    const auto green         = sys_get_le16(light.data() + 4);
    const auto blue          = sys_get_le16(light.data() + 6);

    if (ambient_light_enabled)
    {
        if (!_has_ambient_light_value)
        {
            _ambient_light_filter.reset(ambient_light);
            _last_ambient_light_value = ambient_light;
            _has_ambient_light_value  = true;

            publish_result(_mapper.ambient_light_result(_last_ambient_light_value));
        }
        else
        {
            const auto filtered_ambient_light = _ambient_light_filter.value(ambient_light);

            if (filtered_ambient_light != _last_ambient_light_value)
            {
                _last_ambient_light_value = filtered_ambient_light;
                publish_result(_mapper.ambient_light_result(_last_ambient_light_value));
            }
        }
    }

    if (rgb_enabled)
    {
        if (!_has_rgb_value)
        {
            _rgb_filters[0].reset(ambient_light);
            _rgb_filters[1].reset(red);
            _rgb_filters[2].reset(green);
            _rgb_filters[3].reset(blue);
            _has_rgb_value  = true;
            _last_rgb_value = {
                ambient_light,
                red,
                green,
                blue,
            };

            publish_result(_mapper.rgb_result(_last_rgb_value[0], _last_rgb_value[1], _last_rgb_value[2], _last_rgb_value[3]));
        }
        else
        {
            const std::array<uint16_t, 4> filtered_rgb = {
                _rgb_filters[0].value(ambient_light),
                _rgb_filters[1].value(red),
                _rgb_filters[2].value(green),
                _rgb_filters[3].value(blue),
            };

            if (filtered_rgb != _last_rgb_value)
            {
                _last_rgb_value = filtered_rgb;
                publish_result(_mapper.rgb_result(_last_rgb_value[0], _last_rgb_value[1], _last_rgb_value[2], _last_rgb_value[3]));
            }
        }
    }

    return true;
}

bool SensorApds9960::configure_sensor()
{
    reset_gesture_counts();
    reset_value_states();

    uint8_t enable = APDS9960_ENABLE_PON;

    const auto mode              = proximity_gesture_mode();
    const bool gesture_engine    = mode == ProximityGestureMode::Gesture;
    const bool proximity_enabled = mode == ProximityGestureMode::Proximity;
    const auto ambient_enabled   = output_enabled(Setting::EnableAmbientLight);
    const auto rgb_enabled       = output_enabled(Setting::EnableRgb);

    if (ambient_enabled || rgb_enabled)
    {
        enable |= APDS9960_ENABLE_AEN;
    }

    if (proximity_enabled)
    {
        enable |= APDS9960_ENABLE_PEN;
    }

    if (gesture_engine)
    {
        enable |= APDS9960_ENABLE_PEN | APDS9960_ENABLE_GEN;
    }

    if (!write_register(APDS9960_REGISTER_ENABLE, 0) ||
        !write_register(APDS9960_REGISTER_ATIME, APDS9960_DEFAULT_ATIME) ||
        !write_register(APDS9960_REGISTER_WTIME, APDS9960_DEFAULT_WTIME) ||
        !write_register(APDS9960_REGISTER_PPULSE, APDS9960_DEFAULT_PPULSE) ||
        !write_register(APDS9960_REGISTER_POFFSET_UR, APDS9960_DEFAULT_POFFSET_UR) ||
        !write_register(APDS9960_REGISTER_POFFSET_DL, APDS9960_DEFAULT_POFFSET_DL) ||
        !write_register(APDS9960_REGISTER_CONFIG1, APDS9960_DEFAULT_CONFIG1) ||
        !write_register(APDS9960_REGISTER_CONTROL, control_register_value()) ||
        !write_register(APDS9960_REGISTER_PILT, APDS9960_DEFAULT_PILT) ||
        !write_register(APDS9960_REGISTER_PIHT, APDS9960_DEFAULT_PIHT) ||
        !write_register(APDS9960_REGISTER_AILTL, static_cast<uint8_t>(APDS9960_DEFAULT_AILT & zmisc::BYTE_MASK)) ||
        !write_register(APDS9960_REGISTER_AILTH, static_cast<uint8_t>(APDS9960_DEFAULT_AILT >> zmisc::BYTE_BIT_COUNT)) ||
        !write_register(APDS9960_REGISTER_AIHTL, static_cast<uint8_t>(APDS9960_DEFAULT_AIHT & zmisc::BYTE_MASK)) ||
        !write_register(APDS9960_REGISTER_AIHTH, static_cast<uint8_t>(APDS9960_DEFAULT_AIHT >> zmisc::BYTE_BIT_COUNT)) ||
        !write_register(APDS9960_REGISTER_PERS, APDS9960_DEFAULT_PERS) ||
        !write_register(APDS9960_REGISTER_CONFIG2, APDS9960_DEFAULT_CONFIG2) ||
        !write_register(APDS9960_REGISTER_CONFIG3, APDS9960_DEFAULT_CONFIG3) ||
        !write_register(APDS9960_REGISTER_GPENTH, APDS9960_DEFAULT_GPENTH) ||
        !write_register(APDS9960_REGISTER_GEXTH, APDS9960_DEFAULT_GEXTH) ||
        !write_register(APDS9960_REGISTER_GCONF1, APDS9960_DEFAULT_GCONF1) ||
        !write_register(APDS9960_REGISTER_GCONF2, APDS9960_DEFAULT_GCONF2) ||
        !write_register(APDS9960_REGISTER_GOFFSET_U, APDS9960_DEFAULT_GOFFSET) ||
        !write_register(APDS9960_REGISTER_GOFFSET_D, APDS9960_DEFAULT_GOFFSET) ||
        !write_register(APDS9960_REGISTER_GOFFSET_L, APDS9960_DEFAULT_GOFFSET) ||
        !write_register(APDS9960_REGISTER_GOFFSET_R, APDS9960_DEFAULT_GOFFSET) ||
        !write_register(APDS9960_REGISTER_GPULSE, APDS9960_DEFAULT_GPULSE) ||
        !write_register(APDS9960_REGISTER_GCONF3, APDS9960_DEFAULT_GCONF3) ||
        !write_register(APDS9960_REGISTER_GCONF4, 0))
    {
        return false;
    }

    return write_register(APDS9960_REGISTER_ENABLE, APDS9960_ENABLE_PON) &&
           write_register(APDS9960_REGISTER_ENABLE, enable);
}

bool SensorApds9960::output_enabled(Setting setting)
{
    return _database.read(database::Config::Section::I2c::Apds9960, setting) != 0;
}

ProximityGestureMode SensorApds9960::proximity_gesture_mode()
{
    const auto mode = _database.read(database::Config::Section::I2c::Apds9960, Setting::ProximityGestureMode);

    if (mode >= static_cast<uint32_t>(ProximityGestureMode::Count))
    {
        return ProximityGestureMode::Disabled;
    }

    return static_cast<ProximityGestureMode>(mode);
}

signaling::OscSensorApds9960Gesture SensorApds9960::output_gesture(signaling::OscSensorApds9960Gesture gesture)
{
    const bool invert_x = _database.read(database::Config::Section::I2c::Apds9960, Setting::InvertGestureX) != 0;
    const bool invert_y = _database.read(database::Config::Section::I2c::Apds9960, Setting::InvertGestureY) != 0;

    if (invert_x)
    {
        if (gesture == signaling::OscSensorApds9960Gesture::Left)
        {
            return signaling::OscSensorApds9960Gesture::Right;
        }

        if (gesture == signaling::OscSensorApds9960Gesture::Right)
        {
            return signaling::OscSensorApds9960Gesture::Left;
        }
    }

    if (invert_y)
    {
        if (gesture == signaling::OscSensorApds9960Gesture::Up)
        {
            return signaling::OscSensorApds9960Gesture::Down;
        }

        if (gesture == signaling::OscSensorApds9960Gesture::Down)
        {
            return signaling::OscSensorApds9960Gesture::Up;
        }
    }

    return gesture;
}

uint8_t SensorApds9960::control_register_value()
{
    uint32_t proximity_gain = _database.read(database::Config::Section::I2c::Apds9960, Setting::ProximityGain);
    uint32_t als_gain       = _database.read(database::Config::Section::I2c::Apds9960, Setting::AlsGain);

    if (proximity_gain > 3)
    {
        proximity_gain = APDS9960_DEFAULT_PROXIMITY_GAIN;
    }

    if (als_gain > 3)
    {
        als_gain = APDS9960_DEFAULT_ALS_GAIN;
    }

    return static_cast<uint8_t>((APDS9960_DEFAULT_LED_DRIVE << APDS9960_CONTROL_LED_DRIVE_SHIFT) |
                                (proximity_gain << APDS9960_CONTROL_PROXIMITY_GAIN_SHIFT) |
                                als_gain);
}

bool SensorApds9960::deinit()
{
    if (_found)
    {
        write_register(APDS9960_REGISTER_ENABLE, 0);
    }

    _found                      = false;
    _initialized                = false;
    _read_failure_count         = 0;
    _selected_i2c_address_index = 0;
    reset_value_states();
    reset_gesture_counts();

    return true;
}

uint8_t SensorApds9960::i2c_address() const
{
    return I2C_ADDRESSES.at(_selected_i2c_address_index);
}

bool SensorApds9960::write_register(uint8_t reg, uint8_t value)
{
    uint8_t buffer[] = {
        reg,
        value,
    };

    return _hwa.write(i2c_address(), std::span<const uint8_t>(buffer));
}

std::optional<uint8_t> SensorApds9960::read_register(uint8_t reg)
{
    uint8_t value = 0;

    if (!_hwa.write_read(i2c_address(), std::span<const uint8_t>(&reg, 1), std::span<uint8_t>(&value, 1)))
    {
        record_i2c_failure();
        return {};
    }

    record_i2c_success();

    return value;
}

std::optional<uint16_t> SensorApds9960::read_register_le16(uint8_t reg)
{
    uint8_t buffer[2] = {};

    if (!_hwa.write_read(i2c_address(), std::span<const uint8_t>(&reg, 1), std::span<uint8_t>(buffer)))
    {
        record_i2c_failure();
        return {};
    }

    record_i2c_success();

    return static_cast<uint16_t>(buffer[0]) | (static_cast<uint16_t>(buffer[1]) << zmisc::BYTE_BIT_COUNT);
}

bool SensorApds9960::read_register_block(uint8_t reg, std::span<uint8_t> buffer)
{
    if (!_hwa.write_read(i2c_address(), std::span<const uint8_t>(&reg, 1), buffer))
    {
        record_i2c_failure();
        return false;
    }

    record_i2c_success();

    return true;
}

bool SensorApds9960::recoverable_i2c_read_failure() const
{
    return _initialized;
}

void SensorApds9960::record_i2c_success()
{
    _read_failure_count = 0;
}

void SensorApds9960::record_i2c_failure()
{
    if (!_initialized)
    {
        return;
    }

    if (_read_failure_count < READ_FAILURE_LIMIT)
    {
        _read_failure_count++;
    }

    if (_read_failure_count >= READ_FAILURE_LIMIT)
    {
        mark_disconnected();
    }
}

void SensorApds9960::mark_disconnected()
{
    if (!_initialized)
    {
        return;
    }

    LOG_WRN("APDS9960 disconnected");

    _found                = false;
    _initialized          = false;
    _read_failure_count   = 0;
    _last_gesture_send_ms = 0;
    reset_value_states();
    reset_gesture_counts();
}

std::optional<signaling::OscSensorApds9960Gesture> SensorApds9960::decode_gesture_fifo()
{
    const auto gesture_status = read_register(APDS9960_REGISTER_GSTATUS);

    if (!gesture_status.has_value())
    {
        return {};
    }

    if ((gesture_status.value() & APDS9960_GSTATUS_GVALID) == 0)
    {
        return {};
    }

    auto fifo_level = read_register(APDS9960_REGISTER_GFLVL);

    if (!fifo_level.has_value())
    {
        return {};
    }

    if (fifo_level.value() == 0)
    {
        LOG_DBG("APDS9960 gesture valid but FIFO empty: GSTATUS=0x%02x", gesture_status.value());
        return {};
    }

    if (fifo_level.value() > GESTURE_FIFO_MAX_SAMPLES)
    {
        LOG_WRN("APDS9960 gesture FIFO level invalid: %u", fifo_level.value());
        fifo_level = GESTURE_FIFO_MAX_SAMPLES;
    }

    std::array<uint8_t, GESTURE_FIFO_MAX_SAMPLES * 4U> fifo          = {};
    const size_t                                       bytes_to_read = static_cast<size_t>(fifo_level.value()) * 4U;

    if (!read_register_block(APDS9960_REGISTER_GFIFO_U, std::span<uint8_t>(fifo.data(), bytes_to_read)))
    {
        return {};
    }

    const int64_t now_ms = k_uptime_get();

    if ((_gesture_start_ms != 0) && ((now_ms - _gesture_start_ms) > GESTURE_EDGE_TIMEOUT_MS))
    {
        reset_gesture_edge_state();
    }

    for (size_t i = 0; i < bytes_to_read; i += 4U)
    {
        auto gesture = process_gesture_sample(fifo[i + 0U], fifo[i + 1U], fifo[i + 2U], fifo[i + 3U]);

        if (gesture.has_value())
        {
            return gesture;
        }
    }

    return {};
}

std::optional<signaling::OscSensorApds9960Gesture> SensorApds9960::process_gesture_sample(uint8_t up, uint8_t down, uint8_t left, uint8_t right)
{
    const int64_t now_ms                 = k_uptime_get();
    const int     up_down_delta          = static_cast<int>(up) - static_cast<int>(down);
    const int     left_right_delta       = static_cast<int>(left) - static_cast<int>(right);
    const bool    up_down_significant    = std::abs(up_down_delta) > GESTURE_EDGE_THRESHOLD;
    const bool    left_right_significant = std::abs(left_right_delta) > GESTURE_EDGE_THRESHOLD;

    if (up_down_significant)
    {
        if (up_down_delta < 0)
        {
            if (_gesture_up_started)
            {
                _gesture_up_started   = false;
                _gesture_down_started = false;
                return signaling::OscSensorApds9960Gesture::Up;
            }

            _gesture_down_started = true;
            _gesture_start_ms     = now_ms;
        }
        else
        {
            if (_gesture_down_started)
            {
                _gesture_up_started   = false;
                _gesture_down_started = false;
                return signaling::OscSensorApds9960Gesture::Down;
            }

            _gesture_up_started = true;
            _gesture_start_ms   = now_ms;
        }
    }

    if (left_right_significant)
    {
        if (left_right_delta < 0)
        {
            if (_gesture_left_started)
            {
                _gesture_left_started  = false;
                _gesture_right_started = false;
                return signaling::OscSensorApds9960Gesture::Left;
            }

            _gesture_right_started = true;
            _gesture_start_ms      = now_ms;
        }
        else
        {
            if (_gesture_right_started)
            {
                _gesture_left_started  = false;
                _gesture_right_started = false;
                return signaling::OscSensorApds9960Gesture::Right;
            }

            _gesture_left_started = true;
            _gesture_start_ms     = now_ms;
        }
    }

    return {};
}

void SensorApds9960::reset_gesture_counts()
{
    reset_gesture_edge_state();
}

void SensorApds9960::reset_gesture_edge_state()
{
    _gesture_start_ms      = 0;
    _gesture_up_started    = false;
    _gesture_down_started  = false;
    _gesture_left_started  = false;
    _gesture_right_started = false;
}

void SensorApds9960::reset_value_states()
{
    _has_proximity_value      = false;
    _last_proximity_value     = 0;
    _has_ambient_light_value  = false;
    _last_ambient_light_value = 0;
    _has_rgb_value            = false;
    _last_rgb_value           = {};
    _proximity_filter.reset();
    _ambient_light_filter.reset();

    for (auto& filter : _rgb_filters)
    {
        filter.reset();
    }
}

void SensorApds9960::publish_value_states()
{
    if (!_initialized)
    {
        return;
    }

    if ((proximity_gesture_mode() == ProximityGestureMode::Proximity) && _has_proximity_value)
    {
        publish_result(_mapper.proximity_result(_last_proximity_value));
    }

    if (output_enabled(Setting::EnableAmbientLight) && _has_ambient_light_value)
    {
        publish_result(_mapper.ambient_light_result(_last_ambient_light_value));
    }

    if (output_enabled(Setting::EnableRgb) && _has_rgb_value)
    {
        publish_result(_mapper.rgb_result(_last_rgb_value[0], _last_rgb_value[1], _last_rgb_value[2], _last_rgb_value[3]));
    }
}

void SensorApds9960::publish_result(const Mapper::Result& result) const
{
    signaling::publish(result.osc);
}

std::optional<uint8_t> SensorApds9960::sys_config_get(sys::Config::Section::I2c section, size_t index, uint16_t& value)
{
    if (section != sys::Config::Section::I2c::Apds9960)
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

std::optional<uint8_t> SensorApds9960::sys_config_set(sys::Config::Section::I2c section, size_t index, uint16_t value)
{
    if (section != sys::Config::Section::I2c::Apds9960)
    {
        return {};
    }

    auto init_action = common::InitAction::AsIs;
    auto setting     = static_cast<Setting>(index);

    switch (setting)
    {
    case Setting::ProximityGestureMode:
    {
        if (value >= static_cast<uint16_t>(ProximityGestureMode::Count))
        {
            return sys::Config::Status::ErrorWrite;
        }

        init_action = common::InitAction::Init;
    }
    break;

    case Setting::EnableAmbientLight:
    case Setting::EnableRgb:
    {
        if (value > 1)
        {
            return sys::Config::Status::ErrorWrite;
        }

        init_action = common::InitAction::Init;
    }
    break;

    case Setting::InvertGestureX:
    case Setting::InvertGestureY:
    {
        if (value > 1)
        {
            return sys::Config::Status::ErrorWrite;
        }
    }
    break;

    case Setting::ProximityGain:
    case Setting::AlsGain:
    {
        if (value > 3)
        {
            return sys::Config::Status::ErrorWrite;
        }

        init_action = common::InitAction::Init;
    }
    break;

    case Setting::ProximityLowerValue:
    case Setting::ProximityUpperValue:
        break;

    default:
    {
        return sys::Config::Status::ErrorWrite;
    }
    }

    auto result = _database.update(util::Conversion::sys_2_db_section(section), index, value)
                      ? sys::Config::Status::Ack
                      : sys::Config::Status::ErrorWrite;

    if (result == sys::Config::Status::Ack)
    {
        if (init_action == common::InitAction::Init)
        {
            init(_selected_i2c_address_index);
        }
        else if (init_action == common::InitAction::DeInit)
        {
            deinit();
        }
    }

    return result;
}
