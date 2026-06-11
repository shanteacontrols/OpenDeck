/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/io/i2c/peripherals/sensor_vl53l4cx/instance/impl/deps.h"
#include "firmware/src/io/i2c/peripherals/sensor_vl53l4cx/instance/impl/mapper.h"
#include "firmware/src/signaling/signaling.h"
#include "firmware/src/system/shared/config.h"

#include "vl53l4cx_class.h"

#include <optional>

namespace opendeck::firmware::io::i2c::sensor_vl53l4cx
{
    /**
     * @brief VL53L4CX time-of-flight distance sensor detector.
     */
    class SensorVl53l4cx : public Peripheral
    {
        public:
        /**
         * @brief Constructs the VL53L4CX peripheral bound to the shared I2C backend.
         *
         * @param hwa Hardware abstraction used to communicate with the sensor.
         * @param database Database used to read and update VL53L4CX settings.
         */
        SensorVl53l4cx(Hwa&      hwa,
                       Database& database);

        /**
         * @brief Initializes VL53L4CX at one supported I2C address.
         *
         * @param address_index Index into the supported I2C address list.
         *
         * @return `true` if the sensor was found.
         */
        bool init(size_t address_index) override;

        /**
         * @brief Performs one periodic sensor update.
         *
         * @return `true` while the sensor is measuring, otherwise `false`.
         */
        bool update() override;

        /**
         * @brief Returns the VL53L4CX update interval for the fixed fast timing budget.
         *
         * @return Minimum time between update() calls in milliseconds.
         */
        int64_t update_interval_ms() override;

        /**
         * @brief Deinitializes the sensor runtime state.
         *
         * @return `true` if the sensor was deinitialized.
         */
        bool deinit() override;

        /**
         * @brief Returns the peripheral name used in diagnostics.
         *
         * @return Static peripheral name.
         */
        constexpr std::string_view name() const override;

        /**
         * @brief Returns supported VL53L4CX I2C addresses.
         *
         * @return Candidate 7-bit I2C addresses.
         */
        std::span<const uint8_t> i2c_addresses() const override;

        private:
        Hwa&      _hwa;
        Database& _database;
        Mapper    _mapper;
        VL53L4CX  _driver                     = {};
        bool      _has_distance_value         = false;
        uint16_t  _last_distance_mm           = 0;
        size_t    _selected_i2c_address_index = 0;
        bool      _found                      = false;
        bool      _measuring                  = false;

        /**
         * @brief Applies configured ranging settings.
         *
         * @return `true` if the driver accepted the configuration.
         */
        bool configure_sensor();

        /**
         * @brief Returns true when any distance output is enabled.
         *
         * @return `true` if the sensor should be initialized.
         */
        bool distance_enabled() const;

        /**
         * @brief Returns true when raw millimeter distance output is enabled.
         *
         * @return `true` if raw distance readings should be published.
         */
        bool distance_mm_enabled() const;

        /**
         * @brief Returns true when normalized distance output is enabled.
         *
         * @return `true` if normalized distance readings should be published.
         */
        bool distance_norm_enabled() const;

        /**
         * @brief Returns the configured smoothing profile.
         *
         * @return Valid smoothing profile.
         */
        Smoothing smoothing();

        /**
         * @brief Returns the configured tracking area.
         *
         * @return Valid tracking area value.
         */
        TrackingArea tracking_area();

        /**
         * @brief Returns the configured distance mode.
         *
         * @return Valid distance mode value.
         */
        DistanceMode distance_mode();

        /**
         * @brief Resets the sensor through its software reset register.
         *
         * @return `true` if both reset writes succeeded.
         */
        bool reset_sensor();

        /**
         * @brief Returns the selected VL53L4CX I2C address.
         *
         * @return Selected 7-bit I2C address.
         */
        uint8_t i2c_address() const;

        /**
         * @brief Reads one completed measurement frame.
         */
        void read_measurement_frame();

        /**
         * @brief Processes one completed measurement frame.
         *
         * @param measurement_frame Result structure returned by the ST API.
         */
        void process_measurement_frame(const VL53L4CX_MultiRangingData_t& measurement_frame);

        /**
         * @brief Publishes one mapped VL53L4CX result.
         *
         * @param result Mapper result to publish.
         */
        void publish_result(const Mapper::Result& result) const;

        /**
         * @brief Publishes the last committed distance value.
         */
        void publish_value_state();

        /**
         * @brief Returns whether the status contains usable distance data.
         *
         * @param status Range status code returned by the ST API.
         *
         * @return `true` if distance can be used.
         */
        static bool range_status_usable(uint8_t status);

        /**
         * @brief Applies configured output smoothing to a distance reading.
         *
         * @param distance_mm Raw distance reading in millimeters.
         *
         * @return Smoothed distance in millimeters.
         */
        uint16_t smooth_distance(uint16_t distance_mm);

        /**
         * @brief Serves SysEx configuration reads for the VL53L4CX section.
         *
         * @param section I2C configuration section being read.
         * @param index Parameter index within the section.
         * @param value Output storage for the returned value.
         *
         * @return Status code for handled requests, otherwise `std::nullopt`.
         */
        std::optional<uint8_t> sys_config_get(sys::Config::Section::I2c section, size_t index, uint16_t& value);

        /**
         * @brief Serves SysEx configuration writes for the VL53L4CX section.
         *
         * @param section I2C configuration section being written.
         * @param index Parameter index within the section.
         * @param value Value to write.
         *
         * @return Status code for handled requests, otherwise `std::nullopt`.
         */
        std::optional<uint8_t> sys_config_set(sys::Config::Section::I2c section, size_t index, uint16_t value);
    };
}    // namespace opendeck::firmware::io::i2c::sensor_vl53l4cx
