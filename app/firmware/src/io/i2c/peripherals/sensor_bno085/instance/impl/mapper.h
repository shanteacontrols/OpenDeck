/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/io/i2c/peripherals/sensor_bno085/instance/impl/deps.h"
#include "firmware/src/signaling/signaling.h"

#include <array>
#include <optional>

namespace opendeck::firmware::io::i2c::sensor_bno085
{
    /**
     * @brief Maps raw BNO085 report values into OSC sensor outputs.
     */
    class Mapper
    {
        public:
        /**
         * @brief Result of one mapped BNO085 sensor report.
         */
        struct Result
        {
            std::optional<opendeck::firmware::signaling::OscSensorSignal> quaternion          = {};
            std::optional<opendeck::firmware::signaling::OscSensorSignal> euler               = {};
            std::optional<opendeck::firmware::signaling::OscSensorSignal> gyroscope           = {};
            std::optional<opendeck::firmware::signaling::OscSensorSignal> linear_acceleration = {};
            std::optional<opendeck::firmware::signaling::OscSensorSignal> gravity             = {};
        };

        /**
         * @brief Returns mapped OSC results for one BNO085 report.
         *
         * @param report_id Sensor report ID.
         * @param values Raw report values in x, y, z, w order.
         *
         * @return Mapped OSC sensor result when the report is known, otherwise empty.
         */
        std::optional<Result> result(uint8_t report_id, const std::array<int16_t, 4>& values) const;

        private:
        /**
         * @brief Converts one fixed-point sensor value to float.
         *
         * @param value Raw fixed-point value.
         * @param scale Fixed-point scale factor.
         *
         * @return Normalized value.
         */
        static float normalize(int16_t value, int32_t scale);
    };
}    // namespace opendeck::firmware::io::i2c::sensor_bno085
