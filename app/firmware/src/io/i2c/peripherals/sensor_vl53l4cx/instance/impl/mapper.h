/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/io/i2c/peripherals/sensor_vl53l4cx/instance/impl/deps.h"
#include "firmware/src/signaling/signaling.h"

#include <cstdint>
#include <optional>

namespace opendeck::firmware::io::i2c::sensor_vl53l4cx
{
    /**
     * @brief Maps raw VL53L4CX values into OSC sensor outputs.
     */
    class Mapper
    {
        public:
        /**
         * @brief Result of one mapped VL53L4CX sensor update.
         */
        struct Result
        {
            std::optional<opendeck::firmware::signaling::OscSensorSignal> distance_mm         = {};
            std::optional<opendeck::firmware::signaling::OscSensorSignal> distance_normalized = {};
        };

        /**
         * @brief Creates a mapper backed by the VL53L4CX database settings.
         *
         * @param database Sensor database facade used to read runtime mapping values.
         */
        explicit Mapper(Database& database)
            : _database(database)
        {}

        /**
         * @brief Returns mapped OSC results for one raw distance value.
         *
         * @param distance_mm Distance in millimeters.
         *
         * @return Mapped OSC sensor result when any enabled output changed, otherwise empty.
         */
        std::optional<Result> result(uint16_t distance_mm);

        /**
         * @brief Rebuilds the last published distance result.
         *
         * @return Reconstructed result when one exists, otherwise empty.
         */
        std::optional<Result> last_result() const;

        /**
         * @brief Clears cached duplicate-suppression state.
         */
        void reset();

        private:
        /**
         * @brief Cached mapped output values used for duplicate suppression and refresh replay.
         */
        struct Value
        {
            uint16_t distance_mm               = 0;
            uint16_t distance_normalized       = 0;
            bool     distance_mm_valid         = false;
            bool     distance_normalized_valid = false;
        };

        struct DatabaseInfo
        {
            bool     distance_mm_enabled         = false;
            bool     distance_normalized_enabled = false;
            uint16_t distance_lower_value        = 0;
            uint16_t distance_upper_value        = DISTANCE_MAX_MM;
        };

        Database& _database;
        Value     _last_value = {};

        /**
         * @brief Reads runtime mapping configuration from the database.
         *
         * @return Database-backed runtime info for VL53L4CX value mapping.
         */
        DatabaseInfo read_database_info() const;

        /**
         * @brief Computes the normalized calibrated distance integer value.
         *
         * @param value Raw value.
         * @param lower Lower raw bound.
         * @param upper Upper raw bound.
         *
         * @return Normalized calibrated value in the 0..DISTANCE_MAX_MM range.
         */
        uint16_t compute_value(uint16_t value, uint16_t lower, uint16_t upper) const;

        /**
         * @brief Builds one raw millimeter distance OSC signal.
         *
         * @param distance_mm Distance in millimeters.
         *
         * @return OSC sensor signal.
         */
        opendeck::firmware::signaling::OscSensorSignal distance_mm_signal(uint16_t distance_mm) const;

        /**
         * @brief Builds one normalized distance OSC signal.
         *
         * @param distance_normalized Normalized integer value.
         *
         * @return OSC sensor signal.
         */
        opendeck::firmware::signaling::OscSensorSignal distance_normalized_signal(uint16_t distance_normalized) const;
    };
}    // namespace opendeck::firmware::io::i2c::sensor_vl53l4cx
