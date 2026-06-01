/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "zlibs/utils/misc/string.h"
#include "zlibs/utils/misc/numeric.h"

#include <cstddef>
#include <optional>
#include <string_view>

namespace opendeck::protocol::osc::paths
{
    namespace misc = zlibs::utils::misc;

    constexpr inline auto ROOT              = misc::StringLiteral{ "/opendeck" };
    constexpr inline auto SWITCH_COMPONENT  = misc::StringLiteral{ "/switch" };
    constexpr inline auto ENCODER_COMPONENT = misc::StringLiteral{ "/encoder" };
    constexpr inline auto ANALOG_COMPONENT  = misc::StringLiteral{ "/analog" };
    constexpr inline auto OUTPUT_COMPONENT  = misc::StringLiteral{ "/output" };
    constexpr inline auto SENSORS_COMPONENT = misc::StringLiteral{ "/sensors" };
    constexpr inline auto PROXIMITY         = misc::StringLiteral{ "/proximity" };
    constexpr inline auto AMBIENT_LIGHT     = misc::StringLiteral{ "/ambient_light" };
    constexpr inline auto DISTANCE          = misc::StringLiteral{ "/distance" };
    constexpr inline auto RGB               = misc::StringLiteral{ "/rgb" };
    constexpr inline auto GESTURE           = misc::StringLiteral{ "/gesture" };
    constexpr inline auto IMU               = misc::StringLiteral{ "/imu" };
    constexpr inline auto QUATERNION        = misc::StringLiteral{ "/quaternion" };
    constexpr inline auto EULER             = misc::StringLiteral{ "/euler" };
    constexpr inline auto GYROSCOPE         = misc::StringLiteral{ "/gyro" };
    constexpr inline auto LINEAR_ACCEL      = misc::StringLiteral{ "/linear_accel" };
    constexpr inline auto GRAVITY           = misc::StringLiteral{ "/gravity" };
    constexpr inline auto REFRESH           = misc::StringLiteral{ "/refresh" };
    constexpr inline auto DISCOVER          = misc::StringLiteral{ "/discover" };
    constexpr inline auto DEVICE            = misc::StringLiteral{ "/device" };

    constexpr inline auto SWITCH                  = misc::string_join(ROOT, SWITCH_COMPONENT);
    constexpr inline auto ENCODER                 = misc::string_join(ROOT, ENCODER_COMPONENT);
    constexpr inline auto ANALOG                  = misc::string_join(ROOT, ANALOG_COMPONENT);
    constexpr inline auto OUTPUT                  = misc::string_join(ROOT, OUTPUT_COMPONENT);
    constexpr inline auto SENSORS                 = misc::string_join(ROOT, SENSORS_COMPONENT);
    constexpr inline auto SENSOR_PROXIMITY        = misc::string_join(SENSORS, PROXIMITY);
    constexpr inline auto SENSOR_AMBIENT_LIGHT    = misc::string_join(SENSORS, AMBIENT_LIGHT);
    constexpr inline auto SENSOR_DISTANCE         = misc::string_join(SENSORS, DISTANCE);
    constexpr inline auto SENSOR_RGB              = misc::string_join(SENSORS, RGB);
    constexpr inline auto SENSOR_GESTURE          = misc::string_join(SENSORS, GESTURE);
    constexpr inline auto SENSOR_IMU              = misc::string_join(SENSORS, IMU);
    constexpr inline auto SENSOR_IMU_QUATERNION   = misc::string_join(SENSOR_IMU, QUATERNION);
    constexpr inline auto SENSOR_IMU_EULER        = misc::string_join(SENSOR_IMU, EULER);
    constexpr inline auto SENSOR_IMU_GYROSCOPE    = misc::string_join(SENSOR_IMU, GYROSCOPE);
    constexpr inline auto SENSOR_IMU_LINEAR_ACCEL = misc::string_join(SENSOR_IMU, LINEAR_ACCEL);
    constexpr inline auto SENSOR_IMU_GRAVITY      = misc::string_join(SENSOR_IMU, GRAVITY);
    constexpr inline auto REFRESH_REQ             = misc::string_join(ROOT, REFRESH);
    constexpr inline auto DISCOVERY               = misc::string_join(ROOT, DISCOVER);
    constexpr inline auto DEVICE_INFO             = misc::string_join(ROOT, DEVICE);

    /**
     * @brief Extracts a component index from an address with a fixed OpenDeck path prefix.
     *
     * @param address OSC address pattern.
     * @param prefix Address prefix ending before the component index.
     *
     * @return Component index, or empty when the address does not match.
     */
    inline std::optional<size_t> parse_indexed(std::string_view address, std::string_view prefix)
    {
        if ((address.size() <= (prefix.size() + 1U)) ||
            (address.substr(0, prefix.size()) != prefix) ||
            (address[prefix.size()] != '/'))
        {
            return {};
        }

        size_t index = 0;

        for (size_t i = prefix.size() + 1U; i < address.size(); i++)
        {
            const char c = address[i];

            if ((c < '0') || (c > '9'))
            {
                return {};
            }

            index = (index * misc::DECIMAL_BASE) + static_cast<size_t>(c - '0');
        }

        return index;
    }
}    // namespace opendeck::protocol::osc::paths
