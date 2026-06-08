/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/io/i2c/peripherals/sensor_apds9960/instance/impl/deps.h"
#include "firmware/src/signaling/signaling.h"

#include <cstdint>

namespace opendeck::firmware::io::i2c::sensor_apds9960
{
    /**
     * @brief Maps raw APDS9960 values into OSC sensor values.
     */
    class Mapper
    {
        public:
        /**
         * @brief Result of one mapped APDS9960 sensor update.
         */
        struct Result
        {
            opendeck::firmware::signaling::OscSensorSignal osc = {};
        };

        /**
         * @brief Creates a mapper backed by the APDS9960 database settings.
         *
         * @param database Sensor database facade used to read runtime mapping values.
         */
        explicit Mapper(Database& database)
            : _database(database)
        {}

        /**
         * @brief Returns one mapped result for a raw proximity value.
         *
         * @param value Raw sensor value.
         *
         * @return Mapped OSC sensor result.
         */
        Result proximity_result(uint8_t value) const;

        /**
         * @brief Returns one mapped result for a raw ambient light value.
         *
         * @param value Raw sensor value.
         *
         * @return Mapped OSC sensor result.
         */
        Result ambient_light_result(uint16_t value) const;

        /**
         * @brief Returns one mapped result for raw RGB values.
         *
         * @param red Raw red channel value.
         * @param green Raw green channel value.
         * @param blue Raw blue channel value.
         *
         * @return Mapped OSC sensor result.
         */
        Result rgb_result(uint16_t red, uint16_t green, uint16_t blue) const;

        /**
         * @brief Returns one mapped result for a decoded gesture direction.
         *
         * @param gesture Decoded gesture direction.
         *
         * @return Mapped OSC sensor result.
         */
        Result gesture_result(opendeck::firmware::signaling::OscSensorGesture gesture) const;

        private:
        struct DatabaseInfo
        {
            uint16_t proximity_lower_value = 0;
            uint16_t proximity_upper_value = APDS9960_PROXIMITY_RAW_MAX;
        };

        Database& _database;

        /**
         * @brief Reads runtime mapping configuration from the database.
         *
         * @return Database-backed runtime info for APDS9960 value mapping.
         */
        DatabaseInfo read_database_info() const;

        /**
         * @brief Converts an 8-bit raw proximity value to a scaled 0..255 value.
         *
         * @param value Raw sensor value.
         * @param info Runtime mapping configuration.
         *
         * @return Scaled proximity value.
         */
        uint8_t proximity(uint8_t value, const DatabaseInfo& info) const;

        /**
         * @brief Converts a 16-bit raw RGBC value to a normalized 0..1 float.
         *
         * @param value Raw sensor value.
         * @return Normalized value.
         */
        float rgbc(uint16_t value) const;

        /**
         * @brief Computes the effective mapped 8-bit value.
         *
         * @param value Raw value.
         * @param lower Lower raw bound.
         * @param upper Upper raw bound.
         *
         * @return Mapped 8-bit value.
         */
        uint8_t compute_value(uint16_t value, uint16_t lower, uint16_t upper) const;
    };
}    // namespace opendeck::firmware::io::i2c::sensor_apds9960
