/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "firmware/src/io/i2c/peripherals/sensor_bno085/instance/impl/mapper.h"

#include "zlibs/utils/misc/numeric.h"

#include <cmath>

using namespace opendeck::firmware::io::i2c::sensor_bno085;
using namespace opendeck::firmware;

std::optional<Mapper::Result> Mapper::result(uint8_t report_id, const std::array<int16_t, 4>& values) const
{
    const auto x = values[0];
    const auto y = values[1];
    const auto z = values[2];
    const auto w = values[3];

    Result result = {};

    switch (report_id)
    {
    case ROTATION_VECTOR_REPORT_ID:
    {
        const float real = normalize(w, ROTATION_VECTOR_SCALE);
        const float i    = normalize(x, ROTATION_VECTOR_SCALE);
        const float j    = normalize(y, ROTATION_VECTOR_SCALE);
        const float k    = normalize(z, ROTATION_VECTOR_SCALE);

        result.quaternion.emplace(signaling::OscSensorSignal{
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
        const float pitch = std::asin(zlibs::utils::misc::constrain((-2.0F * ((i * k) - (j * real)) / norm), -1.0F, 1.0F)) *
                            RADIANS_TO_DEGREES;
        const float roll  = std::atan2((2.0F * ((j * k) + (i * real))),
                                       (-i_squared - j_squared + k_squared + real_squared)) *
                            RADIANS_TO_DEGREES;

        result.euler.emplace(signaling::OscSensorSignal{
            .payload = signaling::OscSensorImuEulerSignal{
                .yaw   = yaw,
                .pitch = pitch,
                .roll  = roll,
            },
            .direction = signaling::SignalDirection::Out,
        });
    }
    break;

    case GYROSCOPE_REPORT_ID:
    {
        result.gyroscope.emplace(signaling::OscSensorSignal{
            .payload = signaling::OscSensorImuGyroscopeSignal{
                .x = normalize(x, GYROSCOPE_SCALE),
                .y = normalize(y, GYROSCOPE_SCALE),
                .z = normalize(z, GYROSCOPE_SCALE),
            },
            .direction = signaling::SignalDirection::Out,
        });
    }
    break;

    case LINEAR_ACCEL_REPORT_ID:
    {
        result.linear_acceleration.emplace(signaling::OscSensorSignal{
            .payload = signaling::OscSensorImuLinearAccelerationSignal{
                .x = normalize(x, ACCELERATION_SCALE),
                .y = normalize(y, ACCELERATION_SCALE),
                .z = normalize(z, ACCELERATION_SCALE),
            },
            .direction = signaling::SignalDirection::Out,
        });
    }
    break;

    case GRAVITY_REPORT_ID:
    {
        result.gravity.emplace(signaling::OscSensorSignal{
            .payload = signaling::OscSensorImuGravitySignal{
                .x = normalize(x, ACCELERATION_SCALE),
                .y = normalize(y, ACCELERATION_SCALE),
                .z = normalize(z, ACCELERATION_SCALE),
            },
            .direction = signaling::SignalDirection::Out,
        });
    }
    break;

    default:
        return {};
    }

    return result;
}

float Mapper::normalize(int16_t value, int32_t scale)
{
    return static_cast<float>(value) / static_cast<float>(scale);
}
