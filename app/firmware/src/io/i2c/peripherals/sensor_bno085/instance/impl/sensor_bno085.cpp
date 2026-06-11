/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "firmware/src/io/i2c/peripherals/sensor_bno085/instance/impl/sensor_bno085.h"
#include "firmware/src/io/i2c/instance/impl/i2c.h"
#include "firmware/src/util/configurable/configurable.h"
#include "firmware/src/util/conversion/conversion.h"

#include "zlibs/utils/misc/bit.h"

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

using namespace opendeck::firmware::io::i2c::sensor_bno085;
using namespace opendeck::firmware;
namespace zmisc = zlibs::utils::misc;

namespace
{
    LOG_MODULE_REGISTER(sensor_bno085, CONFIG_OPENDECK_LOG_LEVEL);    // NOLINT

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
}    // namespace

SensorBno085::SensorBno085(Hwa& hwa, Database& database)
    : _hwa(hwa)
    , _database(database)
    , _mapper(database)
{
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

    I2c::register_peripheral(this);
}

bool SensorBno085::init(size_t address_index)
{
    _initialized = false;

    if (address_index >= I2C_ADDRESSES.size())
    {
        return false;
    }

    _selected_i2c_address_index = address_index;

    if (!soft_reset())
    {
        LOG_WRN("BNO085 soft reset failed at address 0x%02x", i2c_address());
        return false;
    }

    k_msleep(SOFT_RESET_DELAY_MS);

    if (!read_startup_packet())
    {
        LOG_WRN("BNO085 startup packet read failed at address 0x%02x", i2c_address());
        return false;
    }

    if (!enable_reports())
    {
        LOG_WRN("BNO085 report setup failed at address 0x%02x", i2c_address());
        return false;
    }

    _initialized = true;
    _has_smoothed_values.fill(false);
    _has_published_values.fill(false);
    _last_publish_ms.fill(0);

    return true;
}

bool SensorBno085::deinit()
{
    _initialized = false;
    return true;
}

bool SensorBno085::update()
{
    if (!_initialized)
    {
        return false;
    }

    return read_report();
}

constexpr std::string_view SensorBno085::name() const
{
    return "sensor_bno085";
}

std::span<const uint8_t> SensorBno085::i2c_addresses() const
{
    return I2C_ADDRESSES;
}

uint8_t SensorBno085::i2c_address() const
{
    return I2C_ADDRESSES[_selected_i2c_address_index];
}

bool SensorBno085::soft_reset()
{
    return _hwa.write(i2c_address(), SHTP_SOFT_RESET);
}

bool SensorBno085::enable_reports()
{
    if ((output_enabled(Setting::EnableQuaternion) || output_enabled(Setting::EnableEuler)) &&
        !enable_report(ROTATION_VECTOR_REPORT_ID))
    {
        LOG_WRN("BNO085 report setup failed: 0x%02x", ROTATION_VECTOR_REPORT_ID);
        return false;
    }

    if (output_enabled(Setting::EnableGyroscope) && !enable_report(GYROSCOPE_REPORT_ID))
    {
        LOG_WRN("BNO085 report setup failed: 0x%02x", GYROSCOPE_REPORT_ID);
        return false;
    }

    if (output_enabled(Setting::EnableLinearAcceleration) && !enable_report(LINEAR_ACCEL_REPORT_ID))
    {
        LOG_WRN("BNO085 report setup failed: 0x%02x", LINEAR_ACCEL_REPORT_ID);
        return false;
    }

    if (output_enabled(Setting::EnableGravity) && !enable_report(GRAVITY_REPORT_ID))
    {
        LOG_WRN("BNO085 report setup failed: 0x%02x", GRAVITY_REPORT_ID);
        return false;
    }

    return true;
}

bool SensorBno085::enable_report(uint8_t report_id)
{
    std::array<uint8_t, SET_FEATURE_COMMAND_SIZE> command = {
        SET_FEATURE_COMMAND_SIZE,
        0x00,
        SHTP_CONTROL_CHANNEL,
        _control_sequence++,
        SHTP_SET_FEATURE_COMMAND,
        report_id,
    };

    write_u32(command, SET_FEATURE_INTERVAL_OFFSET, REPORT_INTERVAL_US);

    const bool written = _hwa.write(i2c_address(), command);

    if (written)
    {
        k_msleep(FEATURE_ENABLE_DELAY_MS);
    }

    return written;
}

bool SensorBno085::output_enabled(Setting setting) const
{
    return _database.read(database::Config::Section::I2c::Bno085, setting) != 0;
}

Smoothing SensorBno085::smoothing() const
{
    const auto value = _database.read(database::Config::Section::I2c::Bno085, Setting::Smoothing);

    if (value >= static_cast<uint8_t>(Smoothing::Count))
    {
        return Smoothing::Off;
    }

    return static_cast<Smoothing>(value);
}

bool SensorBno085::read_startup_packet()
{
    std::array<uint8_t, SHTP_HEADER_SIZE> header = {};

    for (uint8_t attempt = 0; attempt < STARTUP_READ_RETRIES; attempt++)
    {
        if (!_hwa.read(i2c_address(), header))
        {
            k_msleep(STARTUP_READ_DELAY_MS);
            continue;
        }

        const uint16_t size = packet_size(header);

        if ((size < SHTP_HEADER_SIZE) || (size > SHTP_PACKET_SIZE_MAX))
        {
            k_msleep(STARTUP_READ_DELAY_MS);
            continue;
        }

        return true;
    }

    return false;
}

uint16_t SensorBno085::packet_size(std::span<const uint8_t, SHTP_HEADER_SIZE> header)
{
    const uint16_t size = static_cast<uint16_t>(header[0]) |
                          static_cast<uint16_t>(header[1] << 8U);

    return static_cast<uint16_t>(size & ~SHTP_CONTINUATION);
}

bool SensorBno085::read_report()
{
    std::array<uint8_t, SENSOR_REPORT_SIZE> report = {};

    if (!_hwa.read(i2c_address(), report))
    {
        return true;
    }

    publish_report(report);

    return true;
}

void SensorBno085::publish_report(std::span<const uint8_t> report)
{
    const uint8_t                report_id = report[SENSOR_REPORT_ID_OFFSET];
    const int16_t                x         = read_i16(report, SENSOR_REPORT_DATA_OFFSET);
    const int16_t                y         = read_i16(report, SENSOR_REPORT_DATA_OFFSET + 2U);
    const int16_t                z         = read_i16(report, SENSOR_REPORT_DATA_OFFSET + 4U);
    const int16_t                w         = report_id == ROTATION_VECTOR_REPORT_ID ? read_i16(report, SENSOR_REPORT_DATA_OFFSET + 6U) : int16_t{};
    const std::array<int16_t, 4> values    = { x, y, z, w };

    const auto smoothed_values = smooth_values(report_id, values);

    if (!output_changed(report_id, smoothed_values))
    {
        return;
    }

    const auto index = report_index(report_id);

    if (!index.has_value())
    {
        return;
    }

    if (!output_due(report_id))
    {
        return;
    }

    _last_published_values.at(*index) = smoothed_values;
    _has_published_values.at(*index)  = true;

    const auto result = _mapper.result(report_id, smoothed_values);

    if (result.has_value())
    {
        publish_result(result.value());
    }
}

void SensorBno085::publish_result(const Mapper::Result& result) const
{
    if (result.quaternion.has_value())
    {
        signaling::publish(result.quaternion.value());
    }

    if (result.euler.has_value())
    {
        signaling::publish(result.euler.value());
    }

    if (result.gyroscope.has_value())
    {
        signaling::publish(result.gyroscope.value());
    }

    if (result.linear_acceleration.has_value())
    {
        signaling::publish(result.linear_acceleration.value());
    }

    if (result.gravity.has_value())
    {
        signaling::publish(result.gravity.value());
    }
}

std::array<int16_t, 4> SensorBno085::smooth_values(uint8_t report_id, const std::array<int16_t, 4>& values)
{
    const auto index = report_index(report_id);

    if (!index.has_value())
    {
        return values;
    }

    const auto percentage = smoothing_percentage(smoothing());

    if (!_has_smoothed_values.at(*index) || (percentage == SMOOTHING_PERCENTAGE_OFF))
    {
        _has_smoothed_values.at(*index) = true;

        for (size_t i = 0; i < values.size(); i++)
        {
            _value_filters.at(*index).at(i).reset(values.at(i));
        }

        return values;
    }

    std::array<int16_t, 4> smoothed_values = {};

    for (size_t i = 0; i < smoothed_values.size(); i++)
    {
        smoothed_values.at(i) = _value_filters.at(*index).at(i).value(values.at(i), percentage);
    }

    return smoothed_values;
}

bool SensorBno085::output_due(uint8_t report_id)
{
    const auto index = report_index(report_id);

    if (!index.has_value())
    {
        return false;
    }

    auto&      last_publish_ms = _last_publish_ms.at(*index);
    const auto now             = k_uptime_get();

    if (last_publish_ms == 0)
    {
        last_publish_ms = (now == 0) ? 1 : now;
        return true;
    }

    if ((now - last_publish_ms) < OUTPUT_INTERVAL_MS)
    {
        return false;
    }

    last_publish_ms = now;

    return true;
}

bool SensorBno085::output_changed(uint8_t report_id, const std::array<int16_t, 4>& values)
{
    const auto index = report_index(report_id);

    if (!index.has_value())
    {
        return false;
    }

    if (_has_published_values.at(*index) && (_last_published_values.at(*index) == values))
    {
        return false;
    }

    return true;
}

std::optional<uint8_t> SensorBno085::sys_config_get(sys::Config::Section::I2c section, size_t index, uint16_t& value)
{
    if (section != sys::Config::Section::I2c::Bno085)
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

std::optional<uint8_t> SensorBno085::sys_config_set(sys::Config::Section::I2c section, size_t index, uint16_t value)
{
    if (section != sys::Config::Section::I2c::Bno085)
    {
        return {};
    }

    const auto setting = static_cast<Setting>(index);

    switch (setting)
    {
    case Setting::EnableQuaternion:
    case Setting::EnableEuler:
    case Setting::EnableGyroscope:
    case Setting::EnableLinearAcceleration:
    case Setting::EnableGravity:
        if (value > 1U)
        {
            return sys::Config::Status::ErrorWrite;
        }
        break;

    case Setting::Smoothing:
        if (value >= static_cast<uint8_t>(Smoothing::Count))
        {
            return sys::Config::Status::ErrorWrite;
        }
        break;

    default:
        return sys::Config::Status::ErrorWrite;
    }

    auto result = _database.update(util::Conversion::sys_2_db_section(section), index, value)
                      ? sys::Config::Status::Ack
                      : sys::Config::Status::ErrorWrite;

    if ((result == sys::Config::Status::Ack) && (setting == Setting::Smoothing))
    {
        _has_smoothed_values.fill(false);
        _has_published_values.fill(false);
    }
    else if (result == sys::Config::Status::Ack)
    {
        result = init(_selected_i2c_address_index)
                     ? sys::Config::Status::Ack
                     : sys::Config::Status::ErrorWrite;
    }

    return result;
}

int16_t SensorBno085::read_i16(std::span<const uint8_t> report, size_t offset)
{
    const uint16_t value = static_cast<uint16_t>(report[offset]) |
                           static_cast<uint16_t>(report[offset + 1U] << 8U);

    return static_cast<int16_t>(value);
}

void SensorBno085::write_u32(std::span<uint8_t> packet, size_t offset, uint32_t value)
{
    packet[offset]      = static_cast<uint8_t>(value);
    packet[offset + 1U] = static_cast<uint8_t>(value >> zmisc::BYTE_BIT_COUNT);
    packet[offset + 2U] = static_cast<uint8_t>(value >> (zmisc::BYTE_BIT_COUNT * 2U));
    packet[offset + 3U] = static_cast<uint8_t>(value >> (zmisc::BYTE_BIT_COUNT * 3U));
}

std::optional<size_t> SensorBno085::report_index(uint8_t report_id)
{
    switch (report_id)
    {
    case ROTATION_VECTOR_REPORT_ID:
        return 0;

    case GYROSCOPE_REPORT_ID:
        return 1;

    case LINEAR_ACCEL_REPORT_ID:
        return 2;

    case GRAVITY_REPORT_ID:
        return 3;

    default:
        return {};
    }
}
