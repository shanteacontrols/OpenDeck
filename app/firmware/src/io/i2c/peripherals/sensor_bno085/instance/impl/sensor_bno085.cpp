/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "firmware/src/io/i2c/peripherals/sensor_bno085/instance/impl/sensor_bno085.h"
#include "firmware/src/io/i2c/instance/impl/i2c.h"
#include "firmware/src/signaling/signaling.h"

#include "zlibs/utils/misc/bit.h"

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <algorithm>
#include <cmath>

using namespace opendeck::firmware::io::i2c::sensor_bno085;
using namespace opendeck::firmware;
namespace zmisc = zlibs::utils::misc;

namespace
{
    LOG_MODULE_REGISTER(sensor_bno085, CONFIG_OPENDECK_LOG_LEVEL);    // NOLINT

    static constexpr std::array<uint8_t, REPORT_COUNT> REPORTS = {
        ROTATION_VECTOR_REPORT_ID,
        GYROSCOPE_REPORT_ID,
        LINEAR_ACCEL_REPORT_ID,
        GRAVITY_REPORT_ID,
    };
}    // namespace

SensorBno085::SensorBno085(Hwa& hwa)
    : _hwa(hwa)
{
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
    _rotation_vector_filter.reset();
    _gyroscope_filter.reset();
    _linear_accel_filter.reset();
    _gravity_filter.reset();

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
    for (const auto& report : REPORTS)
    {
        if (!enable_report(report))
        {
            LOG_WRN("BNO085 report setup failed: 0x%02x", report);
            return false;
        }

        k_msleep(FEATURE_ENABLE_DELAY_MS);
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

    return _hwa.write(i2c_address(), command);
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

    if (!should_publish(report_id, values))
    {
        return;
    }

    switch (report_id)
    {
    case ROTATION_VECTOR_REPORT_ID:
    {
        const float real = normalize(w, ROTATION_VECTOR_SCALE);
        const float i    = normalize(x, ROTATION_VECTOR_SCALE);
        const float j    = normalize(y, ROTATION_VECTOR_SCALE);
        const float k    = normalize(z, ROTATION_VECTOR_SCALE);

        signaling::publish(signaling::OscSensorSignal{
            .payload = signaling::OscSensorImuQuaternionSignal{
                .real = real,
                .i    = i,
                .j    = j,
                .k    = k,
            },
            .direction = signaling::SignalDirection::Out,
        });

        const float real_squared = real * real;
        const float i_squared    = i * i;
        const float j_squared    = j * j;
        const float k_squared    = k * k;
        const float norm         = real_squared + i_squared + j_squared + k_squared;

        const float yaw   = std::atan2((2.0F * ((i * j) + (k * real))),
                                       (i_squared - j_squared - k_squared + real_squared)) *
                            RADIANS_TO_DEGREES;
        const float pitch = std::asin(std::clamp((-2.0F * ((i * k) - (j * real)) / norm), -1.0F, 1.0F)) *
                            RADIANS_TO_DEGREES;
        const float roll  = std::atan2((2.0F * ((j * k) + (i * real))),
                                       (-i_squared - j_squared + k_squared + real_squared)) *
                            RADIANS_TO_DEGREES;

        signaling::publish(signaling::OscSensorSignal{
            .payload = signaling::OscSensorImuEulerSignal{
                .yaw   = yaw,
                .pitch = pitch,
                .roll  = roll,
            },
            .direction = signaling::SignalDirection::Out,
        });
        break;
    }

    case GYROSCOPE_REPORT_ID:
        signaling::publish(signaling::OscSensorSignal{
            .payload = signaling::OscSensorImuGyroscopeSignal{
                .x = normalize(x, GYROSCOPE_SCALE),
                .y = normalize(y, GYROSCOPE_SCALE),
                .z = normalize(z, GYROSCOPE_SCALE),
            },
            .direction = signaling::SignalDirection::Out,
        });
        break;

    case LINEAR_ACCEL_REPORT_ID:
        signaling::publish(signaling::OscSensorSignal{
            .payload = signaling::OscSensorImuLinearAccelerationSignal{
                .x = normalize(x, ACCELERATION_SCALE),
                .y = normalize(y, ACCELERATION_SCALE),
                .z = normalize(z, ACCELERATION_SCALE),
            },
            .direction = signaling::SignalDirection::Out,
        });
        break;

    case GRAVITY_REPORT_ID:
        signaling::publish(signaling::OscSensorSignal{
            .payload = signaling::OscSensorImuGravitySignal{
                .x = normalize(x, ACCELERATION_SCALE),
                .y = normalize(y, ACCELERATION_SCALE),
                .z = normalize(z, ACCELERATION_SCALE),
            },
            .direction = signaling::SignalDirection::Out,
        });
        break;

    default:
        break;
    }
}

bool SensorBno085::should_publish(uint8_t report_id, const std::array<int16_t, 4>& values)
{
    switch (report_id)
    {
    case ROTATION_VECTOR_REPORT_ID:
        return _rotation_vector_filter.update({ values[0],
                                                values[1],
                                                values[2],
                                                values[3] },
                                              ROTATION_VECTOR_PUBLISH_THRESHOLD,
                                              PUBLISH_CONFIRMATION_SAMPLES,
                                              RotationVectorFilter::ConfirmationMode::Nearby,
                                              ROTATION_VECTOR_PUBLISH_THRESHOLD);

    case GYROSCOPE_REPORT_ID:
        return _gyroscope_filter.update({ values[0],
                                          values[1],
                                          values[2] },
                                        GYROSCOPE_PUBLISH_THRESHOLD,
                                        PUBLISH_CONFIRMATION_SAMPLES,
                                        SensorVectorFilter::ConfirmationMode::Nearby,
                                        GYROSCOPE_PUBLISH_THRESHOLD);

    case LINEAR_ACCEL_REPORT_ID:
        return _linear_accel_filter.update({ values[0],
                                             values[1],
                                             values[2] },
                                           LINEAR_ACCEL_PUBLISH_THRESHOLD,
                                           PUBLISH_CONFIRMATION_SAMPLES,
                                           SensorVectorFilter::ConfirmationMode::Nearby,
                                           LINEAR_ACCEL_PUBLISH_THRESHOLD);

    case GRAVITY_REPORT_ID:
        return _gravity_filter.update({ values[0],
                                        values[1],
                                        values[2] },
                                      GRAVITY_PUBLISH_THRESHOLD,
                                      PUBLISH_CONFIRMATION_SAMPLES,
                                      SensorVectorFilter::ConfirmationMode::Nearby,
                                      GRAVITY_PUBLISH_THRESHOLD);

    default:
        return false;
    }
}

int16_t SensorBno085::read_i16(std::span<const uint8_t> report, size_t offset)
{
    const uint16_t value = static_cast<uint16_t>(report[offset]) |
                           static_cast<uint16_t>(report[offset + 1U] << 8U);

    return static_cast<int16_t>(value);
}

float SensorBno085::normalize(int16_t value, int32_t scale)
{
    return static_cast<float>(value) / static_cast<float>(scale);
}

void SensorBno085::write_u32(std::span<uint8_t> packet, size_t offset, uint32_t value)
{
    packet[offset]      = static_cast<uint8_t>(value);
    packet[offset + 1U] = static_cast<uint8_t>(value >> zmisc::BYTE_BIT_COUNT);
    packet[offset + 2U] = static_cast<uint8_t>(value >> (zmisc::BYTE_BIT_COUNT * 2U));
    packet[offset + 3U] = static_cast<uint8_t>(value >> (zmisc::BYTE_BIT_COUNT * 3U));
}
