/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#ifdef CONFIG_PROJECT_TARGET_SUPPORT_I2C

#include "firmware/src/io/i2c/peripherals/sensor_vl53l4cx/sensor_vl53l4cx.h"
#include "firmware/src/io/i2c/instance/impl/i2c.h"
#include "firmware/src/util/configurable/configurable.h"
#include "firmware/src/util/conversion/conversion.h"

#include "zlibs/utils/misc/bit.h"

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <optional>

using namespace opendeck::io::i2c::sensor_vl53l4cx;
using namespace opendeck;

namespace
{
    LOG_MODULE_REGISTER(sensor_vl53l4cx, CONFIG_OPENDECK_LOG_LEVEL);    // NOLINT

    constexpr uint32_t RESPONSE_FAST_TIMING_BUDGET_US     = 33333;
    constexpr uint32_t RESPONSE_BALANCED_TIMING_BUDGET_US = 66000;
    constexpr uint32_t RESPONSE_STABLE_TIMING_BUDGET_US   = 100000;
    constexpr int32_t  SOFT_RESET_DELAY_MS                = 100;
    constexpr uint8_t  SOFT_RESET_ASSERTED                = 0x00;
    constexpr uint8_t  SOFT_RESET_RELEASED                = 0x01;

    struct Roi
    {
        uint8_t top_left_x;
        uint8_t top_left_y;
        uint8_t bottom_right_x;
        uint8_t bottom_right_y;
    };

    constexpr uint32_t response_timing_budget_us(Response response)
    {
        switch (response)
        {
        case Response::Fast:
            return RESPONSE_FAST_TIMING_BUDGET_US;

        case Response::Balanced:
            return RESPONSE_BALANCED_TIMING_BUDGET_US;

        case Response::Stable:
        default:
            return RESPONSE_STABLE_TIMING_BUDGET_US;
        }
    }

    constexpr VL53L4CX_DistanceModes driver_distance_mode(DistanceMode mode)
    {
        switch (mode)
        {
        case DistanceMode::Long:
            return VL53L4CX_DISTANCEMODE_LONG;

        case DistanceMode::Medium:
        default:
            return VL53L4CX_DISTANCEMODE_MEDIUM;
        }
    }

    constexpr Roi tracking_area_roi(TrackingArea area)
    {
        switch (area)
        {
        case TrackingArea::Medium:
        {
            constexpr Roi ROI = { 4, 11, 11, 4 };
            return ROI;
        }

        case TrackingArea::Wide:
        {
            constexpr Roi ROI = { 2, 13, 13, 2 };
            return ROI;
        }

        case TrackingArea::Full:
        {
            constexpr Roi ROI = { 0, 15, 15, 0 };
            return ROI;
        }

        case TrackingArea::Narrow:
        default:
        {
            constexpr Roi ROI = { 6, 9, 9, 6 };
            return ROI;
        }
        }
    }
}    // namespace

SensorVl53l4cx::SensorVl53l4cx(Hwa&      hwa,
                               Database& database)
    : _hwa(hwa)
    , _database(database)
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

    signaling::subscribe<signaling::ForcedRefreshStart>(
        [this](const signaling::ForcedRefreshStart&)
        {
            publish_value_state();
        });

    _driver.setI2cDevice(&_hwa);
    I2c::register_peripheral(this);
}

bool SensorVl53l4cx::init(size_t address_index)
{
    _found     = false;
    _measuring = false;

    if (address_index >= I2C_ADDRESSES.size())
    {
        return false;
    }

    _selected_i2c_address_index = address_index;

    if (!reset_sensor())
    {
        LOG_DBG("VL53L4CX software reset failed");
        return false;
    }

    uint16_t model_id = 0;

    if (_driver.VL53L4CX_GetSensorId(&model_id) != VL53L4CX_ERROR_NONE)
    {
        LOG_DBG("VL53L4CX responded at 0x%02x, but identity read failed", i2c_address());
        return false;
    }

    const auto init_status = _driver.InitSensor(VL53L4CX_DEFAULT_DEVICE_ADDRESS);

    if (init_status != VL53L4CX_ERROR_NONE)
    {
        LOG_DBG("VL53L4CX init failed: status=%d", init_status);
        return false;
    }

    if (!configure_sensor())
    {
        return false;
    }

    const auto start_status = _driver.VL53L4CX_StartMeasurement();

    if (start_status != VL53L4CX_ERROR_NONE)
    {
        LOG_DBG("VL53L4CX start measurement failed: status=%d", start_status);
        return false;
    }

    _found     = true;
    _measuring = true;
    _distance_filter.reset();

    return true;
}

bool SensorVl53l4cx::reset_sensor()
{
    const std::array<uint8_t, 3> reset_low = {
        static_cast<uint8_t>(VL53L4CX_REGISTER_SOFT_RESET >> zlibs::utils::misc::BYTE_BIT_COUNT),
        static_cast<uint8_t>(VL53L4CX_REGISTER_SOFT_RESET & zlibs::utils::misc::BYTE_MASK),
        SOFT_RESET_ASSERTED,
    };

    if (!_hwa.write(i2c_address(), reset_low))
    {
        return false;
    }

    k_msleep(SOFT_RESET_DELAY_MS);

    const std::array<uint8_t, 3> reset_high = {
        static_cast<uint8_t>(VL53L4CX_REGISTER_SOFT_RESET >> zlibs::utils::misc::BYTE_BIT_COUNT),
        static_cast<uint8_t>(VL53L4CX_REGISTER_SOFT_RESET & zlibs::utils::misc::BYTE_MASK),
        SOFT_RESET_RELEASED,
    };

    if (!_hwa.write(i2c_address(), reset_high))
    {
        return false;
    }

    k_msleep(SOFT_RESET_DELAY_MS);

    return true;
}

bool SensorVl53l4cx::configure_sensor()
{
    const auto configured_distance_mode = driver_distance_mode(distance_mode());
    const auto timing_budget_us         = response_timing_budget_us(response());
    const auto configured_roi           = tracking_area_roi(tracking_area());

    const auto distance_status = _driver.VL53L4CX_SetDistanceMode(configured_distance_mode);

    if (distance_status != VL53L4CX_ERROR_NONE)
    {
        LOG_DBG("VL53L4CX distance mode config failed: status=%d", distance_status);
        return false;
    }

    const auto timing_status = _driver.VL53L4CX_SetMeasurementTimingBudgetMicroSeconds(timing_budget_us);

    if (timing_status != VL53L4CX_ERROR_NONE)
    {
        LOG_DBG("VL53L4CX timing budget config failed: status=%d", timing_status);
        return false;
    }

    VL53L4CX_UserRoi_t user_roi = {
        configured_roi.top_left_x,
        configured_roi.top_left_y,
        configured_roi.bottom_right_x,
        configured_roi.bottom_right_y,
    };

    const auto roi_status = _driver.VL53L4CX_SetUserROI(&user_roi);

    if (roi_status != VL53L4CX_ERROR_NONE)
    {
        LOG_DBG("VL53L4CX ROI config failed: status=%d", roi_status);
        return false;
    }

    const auto xtalk_status = _driver.VL53L4CX_SetXTalkCompensationEnable(0);

    if (xtalk_status != VL53L4CX_ERROR_NONE)
    {
        LOG_DBG("VL53L4CX crosstalk compensation config failed: status=%d", xtalk_status);
        return false;
    }

    const auto smudge_status = _driver.VL53L4CX_SmudgeCorrectionEnable(VL53L4CX_SMUDGE_CORRECTION_NONE);

    if (smudge_status != VL53L4CX_ERROR_NONE)
    {
        LOG_DBG("VL53L4CX smudge correction config failed: status=%d", smudge_status);
        return false;
    }

    return true;
}

bool SensorVl53l4cx::distance_enabled()
{
    return _database.read(database::Config::Section::I2c::Vl53l4cx, Setting::EnableDistance) != 0;
}

TrackingArea SensorVl53l4cx::tracking_area()
{
    const auto value = _database.read(database::Config::Section::I2c::Vl53l4cx, Setting::TrackingArea);

    if (value >= static_cast<uint8_t>(TrackingArea::Count))
    {
        return static_cast<TrackingArea>(VL53L4CX_DEFAULTS[static_cast<uint8_t>(Setting::TrackingArea)]);
    }

    return static_cast<TrackingArea>(value);
}

Response SensorVl53l4cx::response()
{
    const auto value = _database.read(database::Config::Section::I2c::Vl53l4cx, Setting::Response);

    if (value >= static_cast<uint8_t>(Response::Count))
    {
        return static_cast<Response>(VL53L4CX_DEFAULTS[static_cast<uint8_t>(Setting::Response)]);
    }

    return static_cast<Response>(value);
}

DistanceMode SensorVl53l4cx::distance_mode()
{
    const auto value = _database.read(database::Config::Section::I2c::Vl53l4cx, Setting::DistanceMode);

    if (value >= static_cast<uint8_t>(DistanceMode::Count))
    {
        return static_cast<DistanceMode>(VL53L4CX_DEFAULTS[static_cast<uint8_t>(Setting::DistanceMode)]);
    }

    return static_cast<DistanceMode>(value);
}

bool SensorVl53l4cx::deinit()
{
    if (_measuring)
    {
        _driver.VL53L4CX_StopMeasurement();
    }

    _found                      = false;
    _measuring                  = false;
    _selected_i2c_address_index = 0;
    _distance_filter.reset();

    return true;
}

uint8_t SensorVl53l4cx::i2c_address() const
{
    return I2C_ADDRESSES.at(_selected_i2c_address_index);
}

bool SensorVl53l4cx::update()
{
    if (!_found)
    {
        return false;
    }

    read_measurement_frame();

    return _found && _measuring;
}

constexpr std::string_view SensorVl53l4cx::name() const
{
    return "sensor_vl53l4cx";
}

std::span<const uint8_t> SensorVl53l4cx::i2c_addresses() const
{
    return I2C_ADDRESSES;
}

void SensorVl53l4cx::read_measurement_frame()
{
    if (!_measuring)
    {
        return;
    }

    uint8_t data_ready = 0;

    if (_driver.VL53L4CX_GetMeasurementDataReady(&data_ready) != VL53L4CX_ERROR_NONE)
    {
        LOG_WRN("VL53L4CX data-ready read failed");
        _found     = false;
        _measuring = false;
        return;
    }

    if (data_ready == 0)
    {
        return;
    }

    VL53L4CX_MultiRangingData_t measurement_frame = {};

    if (_driver.VL53L4CX_GetMultiRangingData(&measurement_frame) != VL53L4CX_ERROR_NONE)
    {
        LOG_WRN("VL53L4CX measurement frame read failed");
        _found     = false;
        _measuring = false;
        return;
    }

    process_measurement_frame(measurement_frame);

    if (_driver.VL53L4CX_ClearInterruptAndStartMeasurement() != VL53L4CX_ERROR_NONE)
    {
        LOG_WRN("VL53L4CX clear interrupt failed");
        _found     = false;
        _measuring = false;
    }
}

void SensorVl53l4cx::process_measurement_frame(const VL53L4CX_MultiRangingData_t& measurement_frame)
{
    if (!distance_enabled())
    {
        return;
    }

    std::optional<VL53L4CX_TargetRangeData_t> closest;

    for (uint8_t i = 0; i < measurement_frame.NumberOfObjectsFound; i++)
    {
        const auto& object = measurement_frame.RangeData[i];

        if (!range_status_usable(object.RangeStatus) || (object.RangeMilliMeter < 0))
        {
            continue;
        }

        if (!closest.has_value() || (object.RangeMilliMeter < closest.value().RangeMilliMeter))
        {
            closest = object;
        }
    }

    if (!closest.has_value())
    {
        return;
    }

    const auto distance = static_cast<uint16_t>(closest.value().RangeMilliMeter);

    if (_distance_filter.update({ distance },
                                DISTANCE_IDLE_THRESHOLD,
                                DISTANCE_CONFIRMATION_SAMPLES,
                                DISTANCE_MOVING_THRESHOLD))
    {
        publish_distance(closest.value());
    }
}

void SensorVl53l4cx::publish_distance(const VL53L4CX_TargetRangeData_t& object)
{
    signaling::publish(signaling::OscSensorSignal{
        .payload = signaling::OscSensorDistanceSignal{
            .value = object.RangeMilliMeter,
        },
        .direction = signaling::SignalDirection::Out,
    });
}

void SensorVl53l4cx::publish_value_state()
{
    if (!distance_enabled() || !_distance_filter.has_value())
    {
        return;
    }

    signaling::publish(signaling::OscSensorSignal{
        .payload = signaling::OscSensorDistanceSignal{
            .value = _distance_filter.value()[0],
        },
        .direction = signaling::SignalDirection::Out,
    });
}

bool SensorVl53l4cx::range_status_usable(uint8_t status)
{
    return (status == VL53L4CX_RANGESTATUS_RANGE_VALID) ||
           (status == VL53L4CX_RANGESTATUS_RANGE_VALID_MERGED_PULSE);
}

std::optional<uint8_t> SensorVl53l4cx::sys_config_get(sys::Config::Section::I2c section, size_t index, uint16_t& value)
{
    if (section != sys::Config::Section::I2c::Vl53l4cx)
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

std::optional<uint8_t> SensorVl53l4cx::sys_config_set(sys::Config::Section::I2c section, size_t index, uint16_t value)
{
    if (section != sys::Config::Section::I2c::Vl53l4cx)
    {
        return {};
    }

    const auto setting = static_cast<Setting>(index);

    switch (setting)
    {
    case Setting::EnableDistance:
    {
        if (value > 1)
        {
            return sys::Config::Status::ErrorWrite;
        }
    }
    break;

    case Setting::TrackingArea:
    {
        if (value >= static_cast<uint8_t>(TrackingArea::Count))
        {
            return sys::Config::Status::ErrorWrite;
        }
    }
    break;

    case Setting::Response:
    {
        if (value >= static_cast<uint8_t>(Response::Count))
        {
            return sys::Config::Status::ErrorWrite;
        }
    }
    break;

    case Setting::DistanceMode:
    {
        if (value >= static_cast<uint8_t>(DistanceMode::Count))
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

#endif
