/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "firmware/src/io/i2c/peripherals/sensor_cap1188/instance/impl/sensor_cap1188.h"
#include "firmware/src/io/i2c/instance/impl/i2c.h"
#include "firmware/src/signaling/signaling.h"
#include "firmware/src/util/configurable/configurable.h"
#include "firmware/src/util/conversion/conversion.h"

#include "zlibs/utils/misc/bit.h"

#include <zephyr/logging/log.h>

#include <array>

using namespace opendeck::firmware::io::i2c::sensor_cap1188;
using namespace opendeck::firmware;

namespace
{
    LOG_MODULE_REGISTER(sensor_cap1188, CONFIG_OPENDECK_LOG_LEVEL);    // NOLINT

    constexpr uint8_t sensitivity_value(Sensitivity sensitivity)
    {
        switch (sensitivity)
        {
        case Sensitivity::High:
            return CAP1188_DELTA_SENSE_64X;

        case Sensitivity::Low:
            return CAP1188_DELTA_SENSE_16X;

        case Sensitivity::Medium:
        default:
            return CAP1188_DELTA_SENSE_32X;
        }
    }
}    // namespace

SensorCap1188::SensorCap1188(Hwa&      hwa,
                             Database& database)
    : _hwa(hwa)
    , _database(database)
{
    _database.register_layout_init_provider(
        database::Config::Section::I2c::Cap1188,
        [](size_t index) -> std::optional<uint32_t>
        {
            switch (static_cast<Setting>(index))
            {
            case Setting::Sensitivity:
                return static_cast<uint32_t>(Sensitivity::Medium);

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
            publish_cached_states();
        });

    I2c::register_peripheral(this);
}

bool SensorCap1188::init(size_t address_index)
{
    _initialized = false;

    if (address_index >= I2C_ADDRESSES.size())
    {
        return false;
    }

    _selected_i2c_address_index = address_index;

    const auto product_id = read_register(CAP1188_REGISTER_PRODUCT_ID);

    if (!product_id.has_value())
    {
        LOG_DBG("CAP1188 responded at 0x%02x, but product ID read failed", i2c_address());
        return false;
    }

    if (product_id.value() != CAP1188_PRODUCT_ID)
    {
        LOG_DBG("CAP1188 candidate at 0x%02x has unexpected product ID 0x%02x",
                i2c_address(),
                product_id.value());
        return false;
    }

    const auto manufacturer_id = read_register(CAP1188_REGISTER_MANUFACTURER_ID);

    if (!manufacturer_id.has_value())
    {
        LOG_DBG("CAP1188 responded at 0x%02x, but manufacturer ID read failed", i2c_address());
        return false;
    }

    if (manufacturer_id.value() != CAP1188_MANUFACTURER_ID)
    {
        LOG_DBG("CAP1188 candidate at 0x%02x has unexpected manufacturer ID 0x%02x",
                i2c_address(),
                manufacturer_id.value());
        return false;
    }

    const auto revision = read_register(CAP1188_REGISTER_REVISION);

    if (!revision.has_value())
    {
        LOG_DBG("CAP1188 responded at 0x%02x, but revision read failed", i2c_address());
        return false;
    }

    if (!clear_interrupt())
    {
        LOG_DBG("CAP1188 interrupt clear failed at 0x%02x", i2c_address());
        return false;
    }

    if (!configure_sensitivity())
    {
        LOG_DBG("CAP1188 sensitivity config failed at 0x%02x", i2c_address());
        return false;
    }

    if (!configure_led_links())
    {
        LOG_DBG("CAP1188 LED link config failed at 0x%02x", i2c_address());
        return false;
    }

    _last_sensor_input_status = 0;
    _initialized              = true;

    return true;
}

bool SensorCap1188::deinit()
{
    _initialized = false;
    return true;
}

bool SensorCap1188::update()
{
    if (!_initialized)
    {
        return false;
    }

    const auto sensor_input_status = read_register(CAP1188_REGISTER_SENSOR_INPUT);

    if (!sensor_input_status.has_value())
    {
        LOG_WRN("CAP1188 sensor input read failed at 0x%02x", i2c_address());
        return false;
    }

    handle_input_status(sensor_input_status.value());

    if (!clear_interrupt())
    {
        LOG_WRN("CAP1188 interrupt clear failed at 0x%02x", i2c_address());
        return false;
    }

    return true;
}

constexpr std::string_view SensorCap1188::name() const
{
    return "sensor_cap1188";
}

std::span<const uint8_t> SensorCap1188::i2c_addresses() const
{
    return I2C_ADDRESSES;
}

uint8_t SensorCap1188::i2c_address() const
{
    return I2C_ADDRESSES[_selected_i2c_address_index];
}

std::optional<uint8_t> SensorCap1188::read_register(uint8_t reg)
{
    std::array<uint8_t, 1> write_buffer = { reg };
    std::array<uint8_t, 1> read_buffer  = {};

    if (!_hwa.write_read(i2c_address(), write_buffer, read_buffer))
    {
        return {};
    }

    return read_buffer[0];
}

bool SensorCap1188::write_register(uint8_t reg, uint8_t value)
{
    const std::array<uint8_t, 2> buffer = { reg, value };
    return _hwa.write(i2c_address(), buffer);
}

bool SensorCap1188::configure_led_links()
{
    return write_register(CAP1188_REGISTER_LED_LINKING, CAP1188_LED_LINK_ALL_INPUTS);
}

bool SensorCap1188::configure_sensitivity()
{
    const auto register_value = read_register(CAP1188_REGISTER_SENSITIVITY);

    if (!register_value.has_value())
    {
        return false;
    }

    return write_register(CAP1188_REGISTER_SENSITIVITY,
                          static_cast<uint8_t>((register_value.value() & ~CAP1188_SENSITIVITY_MASK) |
                                               sensitivity_value(sensitivity())));
}

Sensitivity SensorCap1188::sensitivity()
{
    const auto value = _database.read(database::Config::Section::I2c::Cap1188, Setting::Sensitivity);

    if (value >= static_cast<uint8_t>(Sensitivity::Count))
    {
        return Sensitivity::Medium;
    }

    return static_cast<Sensitivity>(value);
}

void SensorCap1188::handle_input_status(uint8_t status)
{
    const auto changed = static_cast<uint8_t>(status ^ _last_sensor_input_status);

    if (changed == 0)
    {
        return;
    }

    for (uint8_t index = 0; index < CAP1188_SENSOR_INPUT_COUNT; index++)
    {
        if (!zlibs::utils::misc::bit_read(changed, index))
        {
            continue;
        }

        publish_touch(index, zlibs::utils::misc::bit_read(status, index));
    }

    _last_sensor_input_status = status;
}

void SensorCap1188::publish_cached_states()
{
    if (!_initialized)
    {
        return;
    }

    for (uint8_t index = 0; index < CAP1188_SENSOR_INPUT_COUNT; index++)
    {
        publish_touch(index, zlibs::utils::misc::bit_read(_last_sensor_input_status, index));
    }
}

void SensorCap1188::publish_touch(size_t index, bool touched)
{
    signaling::publish(signaling::OscSensorSignal{
        .payload = signaling::OscSensorTouchSignal{
            .index = index,
            .value = touched ? 1 : 0,
        },
        .direction = signaling::SignalDirection::Out,
    });
}

bool SensorCap1188::clear_interrupt()
{
    const auto main_control = read_register(CAP1188_REGISTER_MAIN_CONTROL);

    if (!main_control.has_value())
    {
        return false;
    }

    if ((main_control.value() & CAP1188_MAIN_CONTROL_INTERRUPT) == 0)
    {
        return true;
    }

    return write_register(CAP1188_REGISTER_MAIN_CONTROL,
                          static_cast<uint8_t>(main_control.value() & ~CAP1188_MAIN_CONTROL_INTERRUPT));
}

std::optional<uint8_t> SensorCap1188::sys_config_get(sys::Config::Section::I2c section, size_t index, uint16_t& value)
{
    if (section != sys::Config::Section::I2c::Cap1188)
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

std::optional<uint8_t> SensorCap1188::sys_config_set(sys::Config::Section::I2c section, size_t index, uint16_t value)
{
    if (section != sys::Config::Section::I2c::Cap1188)
    {
        return {};
    }

    const auto setting = static_cast<Setting>(index);

    switch (setting)
    {
    case Setting::Sensitivity:
    {
        if (value >= static_cast<uint8_t>(Sensitivity::Count))
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

    if (result == sys::Config::Status::Ack)
    {
        init(_selected_i2c_address_index);
    }

    return result;
}
