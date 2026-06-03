/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/io/i2c/peripherals/sensor_cap1188/instance/impl/deps.h"
#include "firmware/src/system/shared/config.h"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string_view>

namespace opendeck::firmware::io::i2c::sensor_cap1188
{
    /**
     * @brief CAP1188 capacitive touch sensor detector.
     */
    class SensorCap1188 : public Peripheral
    {
        public:
        /**
         * @brief Constructs the CAP1188 peripheral bound to the shared I2C backend.
         *
         * @param hwa Hardware abstraction used to communicate with the sensor.
         * @param database Database used to read and update CAP1188 settings.
         */
        SensorCap1188(Hwa&      hwa,
                      Database& database);

        /**
         * @brief Initializes CAP1188 at one supported I2C address.
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
         * @brief Performs one periodic sensor update.
         *
         * @return `true` while the sensor is initialized, otherwise `false`.
         */
        bool update() override;

        /**
         * @brief Returns the peripheral name used in diagnostics.
         *
         * @return Static peripheral name.
         */
        constexpr std::string_view name() const override;

        /**
         * @brief Returns supported CAP1188 I2C addresses.
         *
         * @return Candidate 7-bit I2C addresses.
         */
        std::span<const uint8_t> i2c_addresses() const override;

        private:
        Hwa&      _hwa;
        Database& _database;
        bool      _initialized                = false;
        size_t    _selected_i2c_address_index = 0;
        uint8_t   _last_sensor_input_status   = 0;

        /**
         * @brief Returns the selected CAP1188 I2C address.
         *
         * @return Selected 7-bit I2C address.
         */
        uint8_t i2c_address() const;

        /**
         * @brief Reads one CAP1188 register.
         *
         * @param reg Register address.
         *
         * @return Register value, or empty if the read failed.
         */
        std::optional<uint8_t> read_register(uint8_t reg);

        /**
         * @brief Writes one CAP1188 register.
         *
         * @param reg Register address.
         * @param value Value to write.
         *
         * @return `true` if the write succeeded.
         */
        bool write_register(uint8_t reg, uint8_t value);

        /**
         * @brief Links LED1-LED8 to CS1-CS8 so the chip mirrors touched inputs.
         *
         * @return `true` if the LED link register was written.
         */
        bool configure_led_links();

        /**
         * @brief Applies the configured CAP1188 touch sensitivity.
         *
         * @return `true` if the sensitivity register was written.
         */
        bool configure_sensitivity();

        /**
         * @brief Returns the configured touch sensitivity preset.
         *
         * @return Valid sensitivity value.
         */
        Sensitivity sensitivity();

        /**
         * @brief Processes one sensor-input status read and publishes changed inputs.
         *
         * @param status Current CAP1188 input status bitfield.
         */
        void handle_input_status(uint8_t status);

        /**
         * @brief Publishes the cached state of all CAP1188 inputs.
         */
        void publish_cached_states();

        /**
         * @brief Publishes one CAP1188 input state.
         *
         * @param index Zero-based input index.
         * @param touched Whether the input is currently touched.
         */
        void publish_touch(size_t index, bool touched);

        /**
         * @brief Clears the CAP1188 interrupt latch if it is set.
         *
         * @return `true` if the interrupt register was read and, when needed, written.
         */
        bool clear_interrupt();

        /**
         * @brief Serves SysEx configuration reads for the CAP1188 section.
         *
         * @param section I2C configuration section being read.
         * @param index Parameter index within the section.
         * @param value Output storage for the returned value.
         *
         * @return Status code for handled requests, otherwise `std::nullopt`.
         */
        std::optional<uint8_t> sys_config_get(sys::Config::Section::I2c section, size_t index, uint16_t& value);

        /**
         * @brief Serves SysEx configuration writes for the CAP1188 section.
         *
         * @param section I2C configuration section being written.
         * @param index Parameter index within the section.
         * @param value Value to write.
         *
         * @return Status code for handled requests, otherwise `std::nullopt`.
         */
        std::optional<uint8_t> sys_config_set(sys::Config::Section::I2c section, size_t index, uint16_t value);
    };
}    // namespace opendeck::firmware::io::i2c::sensor_cap1188
