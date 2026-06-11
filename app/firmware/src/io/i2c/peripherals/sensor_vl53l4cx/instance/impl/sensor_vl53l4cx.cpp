/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "firmware/src/io/i2c/peripherals/sensor_vl53l4cx/instance/impl/sensor_vl53l4cx.h"
#include "firmware/src/io/i2c/instance/impl/i2c.h"
#include "firmware/src/util/configurable/configurable.h"
#include "firmware/src/util/conversion/conversion.h"

#include "zlibs/utils/misc/bit.h"

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <optional>

using namespace opendeck::firmware::io::i2c::sensor_vl53l4cx;
using namespace opendeck::firmware;

namespace
{
    LOG_MODULE_REGISTER(sensor_vl53l4cx, CONFIG_OPENDECK_LOG_LEVEL);    // NOLINT

    constexpr uint32_t TIMING_BUDGET_US             = 33333;
    constexpr uint32_t SCAN_MARGIN_US               = 2000;
    constexpr uint32_t MICROSECONDS_PER_MILLISECOND = 1000;
    constexpr int64_t  UPDATE_INTERVAL_MS           = (TIMING_BUDGET_US + SCAN_MARGIN_US) / MICROSECONDS_PER_MILLISECOND;
    constexpr int32_t  SOFT_RESET_DELAY_MS          = 100;
    constexpr uint8_t  SOFT_RESET_ASSERTED          = 0x00;
    constexpr uint8_t  SOFT_RESET_RELEASED          = 0x01;
    constexpr uint8_t  SMOOTHING_PERCENTAGE_OFF     = 100;
    constexpr uint8_t  SMOOTHING_PERCENTAGE_LIGHT   = 55;
    constexpr uint8_t  SMOOTHING_PERCENTAGE_MEDIUM  = 30;
    constexpr uint8_t  SMOOTHING_PERCENTAGE_HEAVY   = 15;

    struct Roi
    {
        uint8_t top_left_x;
        uint8_t top_left_y;
        uint8_t bottom_right_x;
        uint8_t bottom_right_y;
    };

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
    , _mapper(database)
{
    _database.register_layout_init_provider(
        database::Config::Section::I2c::Vl53l4cx,
        [](size_t index) -> std::optional<uint32_t>
        {
            switch (static_cast<Setting>(index))
            {
            case Setting::Smoothing:
                return static_cast<uint32_t>(Smoothing::Heavy);

            case Setting::TrackingArea:
                return static_cast<uint32_t>(TrackingArea::Narrow);

            case Setting::DistanceMode:
                return static_cast<uint32_t>(DistanceMode::Medium);

            case Setting::DistanceUpperValue:
                return DISTANCE_MAX_MM;

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

    if (model_id != VL53L4CX_EXPECTED_SENSOR_ID)
    {
        LOG_DBG("VL53L4CX responded at 0x%02x with unexpected identity 0x%04x", i2c_address(), model_id);
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

    _found              = true;
    _measuring          = true;
    _has_distance_value = false;
    _last_distance_mm   = 0;
    _mapper.reset();

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
    const auto configured_roi           = tracking_area_roi(tracking_area());

    const auto distance_status = _driver.VL53L4CX_SetDistanceMode(configured_distance_mode);

    if (distance_status != VL53L4CX_ERROR_NONE)
    {
        LOG_DBG("VL53L4CX distance mode config failed: status=%d", distance_status);
        return false;
    }

    const auto timing_status = _driver.VL53L4CX_SetMeasurementTimingBudgetMicroSeconds(TIMING_BUDGET_US);

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

bool SensorVl53l4cx::distance_enabled() const
{
    return distance_mm_enabled() || distance_norm_enabled();
}

bool SensorVl53l4cx::distance_mm_enabled() const
{
    return _database.read(database::Config::Section::I2c::Vl53l4cx, Setting::EnableDistanceMm) != 0;
}

bool SensorVl53l4cx::distance_norm_enabled() const
{
    return _database.read(database::Config::Section::I2c::Vl53l4cx, Setting::EnableDistanceNorm) != 0;
}

Smoothing SensorVl53l4cx::smoothing()
{
    const auto value = _database.read(database::Config::Section::I2c::Vl53l4cx, Setting::Smoothing);

    if (value >= static_cast<uint8_t>(Smoothing::Count))
    {
        return Smoothing::Off;
    }

    return static_cast<Smoothing>(value);
}

TrackingArea SensorVl53l4cx::tracking_area()
{
    const auto value = _database.read(database::Config::Section::I2c::Vl53l4cx, Setting::TrackingArea);

    if (value >= static_cast<uint8_t>(TrackingArea::Count))
    {
        return TrackingArea::Narrow;
    }

    return static_cast<TrackingArea>(value);
}

DistanceMode SensorVl53l4cx::distance_mode()
{
    const auto value = _database.read(database::Config::Section::I2c::Vl53l4cx, Setting::DistanceMode);

    if (value >= static_cast<uint8_t>(DistanceMode::Count))
    {
        return DistanceMode::Medium;
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
    _has_distance_value         = false;
    _last_distance_mm           = 0;
    _mapper.reset();
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

int64_t SensorVl53l4cx::update_interval_ms()
{
    return UPDATE_INTERVAL_MS;
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

    if (!_has_distance_value)
    {
        _last_distance_mm   = distance;
        _has_distance_value = true;
    }
    else
    {
        const auto filtered_distance = smooth_distance(distance);

        if (filtered_distance == _last_distance_mm)
        {
            return;
        }

        _last_distance_mm = filtered_distance;
    }

    const auto result = _mapper.result(_last_distance_mm);

    if (result.has_value())
    {
        publish_result(result.value());
    }
}

void SensorVl53l4cx::publish_result(const Mapper::Result& result) const
{
    if (result.distance_mm.has_value())
    {
        signaling::publish(result.distance_mm.value());
    }

    if (result.distance_normalized.has_value())
    {
        signaling::publish(result.distance_normalized.value());
    }
}

void SensorVl53l4cx::publish_value_state()
{
    if (!distance_enabled() || !_has_distance_value)
    {
        return;
    }

    const auto result = _mapper.last_result();

    if (result.has_value())
    {
        publish_result(result.value());
    }
}

bool SensorVl53l4cx::range_status_usable(uint8_t status)
{
    return (status == VL53L4CX_RANGESTATUS_RANGE_VALID) ||
           (status == VL53L4CX_RANGESTATUS_RANGE_VALID_MERGED_PULSE);
}

uint16_t SensorVl53l4cx::smooth_distance(uint16_t distance_mm)
{
    const auto percentage = smoothing_percentage(smoothing());

    if (!_has_distance_value || (percentage == SMOOTHING_PERCENTAGE_OFF))
    {
        _distance_filter.reset(distance_mm);
        return distance_mm;
    }

    return _distance_filter.value(distance_mm, percentage);
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
    case Setting::EnableDistanceMm:
    case Setting::EnableDistanceNorm:
    {
        if (value > 1)
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

    case Setting::TrackingArea:
    {
        if (value >= static_cast<uint8_t>(TrackingArea::Count))
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

    case Setting::DistanceLowerValue:
    case Setting::DistanceUpperValue:
        break;

    default:
        return sys::Config::Status::ErrorWrite;
    }

    auto result = _database.update(util::Conversion::sys_2_db_section(section), index, value)
                      ? sys::Config::Status::Ack
                      : sys::Config::Status::ErrorWrite;

    if ((result == sys::Config::Status::Ack) &&
        ((setting == Setting::TrackingArea) ||
         (setting == Setting::DistanceMode)))
    {
        init(_selected_i2c_address_index);
    }

    return result;
}
