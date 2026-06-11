/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/io/i2c/peripherals/sensor_vl53l5cx/instance/impl/deps.h"
#include "firmware/src/signaling/signaling.h"
#include "firmware/src/system/shared/config.h"

#include "zlibs/utils/filters/filters.h"

#include "vl53l5cx_class.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string_view>

namespace opendeck::firmware::io::i2c::sensor_vl53l5cx
{
    /**
     * @brief VL53L5CX time-of-flight sensor detector.
     */
    class SensorVl53l5cx : public Peripheral
    {
        public:
        /**
         * @brief Constructs the VL53L5CX peripheral bound to the shared I2C backend.
         *
         * @param hwa Hardware abstraction used to communicate with the sensor.
         * @param database Database used to read and update VL53L5CX settings.
         */
        SensorVl53l5cx(Hwa&      hwa,
                       Database& database);

        /**
         * @brief Initializes VL53L5CX at one supported I2C address.
         *
         * @param address_index Index into the supported I2C address list.
         *
         * @return `true` if the device identity was verified.
         */
        bool init(size_t address_index) override;

        /**
         * @brief Deinitializes the sensor runtime state.
         *
         * @return `true` if the sensor was deinitialized.
         */
        bool deinit() override;

        /**
         * @brief Keeps the detected sensor registered with the I2C manager.
         *
         * @return `true` while the sensor is initialized, otherwise `false`.
         */
        bool update() override;

        /**
         * @brief Returns the VL53L5CX polling interval.
         *
         * @return Minimum time between update() calls in milliseconds.
         */
        int64_t update_interval_ms() override;

        /**
         * @brief Returns the peripheral name used in diagnostics.
         *
         * @return Static peripheral name.
         */
        constexpr std::string_view name() const override
        {
            return "sensor_vl53l5cx";
        }

        /**
         * @brief Returns supported VL53L5CX I2C addresses.
         *
         * @return Candidate 7-bit I2C addresses.
         */
        std::span<const uint8_t> i2c_addresses() const override;

        private:
        static constexpr size_t MAX_ZONE_COUNT = 64;

        using DistanceFilter = zlibs::utils::filters::EmaFilter<uint16_t>;

        struct Zone
        {
            int32_t distance_mm = 0;
            bool    valid       = false;
        };

        struct Point
        {
            size_t x = 0;
            size_t y = 0;
        };

        Hwa&                                       _hwa;
        Database&                                  _database;
        VL53L5CX                                   _driver;
        VL53L5CX_ResultsData                       _results                    = {};
        std::array<Zone, MAX_ZONE_COUNT>           _grid                       = {};
        std::array<DistanceFilter, MAX_ZONE_COUNT> _distance_filters           = {};
        std::array<uint8_t, MAX_ZONE_COUNT>        _invalid_frame_count        = {};
        std::array<bool, MAX_ZONE_COUNT>           _has_smoothed_distance      = {};
        bool                                       _initialized                = false;
        bool                                       _ranging                    = false;
        size_t                                     _selected_i2c_address_index = 0;
        int64_t                                    _last_publish_ms            = 0;

        /**
         * @brief Returns the configured ranging resolution.
         *
         * @return Valid resolution value.
         */
        Resolution resolution() const;

        /**
         * @brief Returns the configured smoothing profile.
         *
         * @return Valid smoothing profile.
         */
        Smoothing smoothing() const;

        /**
         * @brief Returns the configured OSC output mode.
         *
         * @return Valid output mode.
         */
        OutputMode output_mode() const;

        /**
         * @brief Returns the configured lower distance gate.
         *
         * @return Lower distance gate in millimeters.
         */
        uint16_t distance_lower_value() const;

        /**
         * @brief Returns the configured upper distance gate.
         *
         * @return Upper distance gate in millimeters.
         */
        uint16_t distance_upper_value() const;

        /**
         * @brief Returns whether OSC output X coordinates are inverted.
         *
         * @return `true` if X is inverted.
         */
        bool invert_x() const;

        /**
         * @brief Returns whether OSC output Y coordinates are inverted.
         *
         * @return `true` if Y is inverted.
         */
        bool invert_y() const;

        /**
         * @brief Returns the configured OSC output rotation.
         *
         * @return Valid rotation value.
         */
        Rotation rotation() const;

        /**
         * @brief Returns the configured OSC output rate limit.
         *
         * @return Valid output rate value.
         */
        OutputRate output_rate() const;

        /**
         * @brief Resets application-side frame processing state.
         */
        void reset_processing_state();

        /**
         * @brief Applies validity, distance window, smoothing, and orientation to the latest raw frame.
         */
        void process_results();

        /**
         * @brief Publishes the processed frame in the configured OSC output mode.
         */
        void publish_results();

        /**
         * @brief Publishes the processed frame as row packets.
         *
         * @param width Active grid width.
         */
        void publish_grid(size_t width) const;

        /**
         * @brief Publishes the nearest valid zone.
         *
         * @param width Active grid width.
         */
        void publish_nearest(size_t width) const;

        /**
         * @brief Publishes the centroid of valid zones.
         *
         * @param width Active grid width.
         */
        void publish_centroid(size_t width) const;

        /**
         * @brief Publishes compact presence data.
         *
         * @param width Active grid width.
         */
        void publish_presence(size_t width) const;

        /**
         * @brief Returns the oriented output coordinate for a source grid coordinate.
         *
         * @param x Source X coordinate.
         * @param y Source Y coordinate.
         * @param width Active grid width.
         *
         * @return Oriented output coordinate.
         */
        Point oriented_point(size_t x, size_t y, size_t width) const;

        /**
         * @brief Applies the configured smoothing profile to one zone.
         *
         * @param zone Source zone index.
         * @param distance_mm Current raw distance in millimeters.
         *
         * @return Filtered distance in millimeters.
         */
        uint16_t smooth_distance(size_t zone, uint16_t distance_mm);

        /**
         * @brief Serves SysEx configuration reads for the VL53L5CX section.
         *
         * @param section I2C configuration section being read.
         * @param index Parameter index within the section.
         * @param value Output storage for the returned value.
         *
         * @return Status code for handled requests, otherwise `std::nullopt`.
         */
        std::optional<uint8_t> sys_config_get(sys::Config::Section::I2c section, size_t index, uint16_t& value);

        /**
         * @brief Serves SysEx configuration writes for the VL53L5CX section.
         *
         * @param section I2C configuration section being written.
         * @param index Parameter index within the section.
         * @param value Value to write.
         *
         * @return Status code for handled requests, otherwise `std::nullopt`.
         */
        std::optional<uint8_t> sys_config_set(sys::Config::Section::I2c section, size_t index, uint16_t value);

        /**
         * @brief Returns the selected VL53L5CX I2C address.
         *
         * @return Selected 7-bit I2C address.
         */
        uint8_t i2c_address() const;

        /**
         * @brief Reads one VL53L5CX register.
         *
         * @param reg 16-bit register address.
         *
         * @return Register value, or empty if the read failed.
         */
        std::optional<uint8_t> read_register(uint16_t reg);

        /**
         * @brief Writes one VL53L5CX register.
         *
         * @param reg 16-bit register address.
         * @param value Value to write.
         *
         * @return `true` if the write succeeded.
         */
        bool write_register(uint16_t reg, uint8_t value);

        /**
         * @brief Selects one VL53L5CX register page.
         *
         * @param page Page selector value.
         *
         * @return `true` if the page-select register was written.
         */
        bool select_page(uint8_t page);

        /**
         * @brief Verifies the VL53L5CX device and revision IDs.
         *
         * @return `true` if both identity bytes match the expected values.
         */
        bool verify_identity();
    };
}    // namespace opendeck::firmware::io::i2c::sensor_vl53l5cx
