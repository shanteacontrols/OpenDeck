/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/io/i2c/peripherals/sensor_bno085/instance/impl/deps.h"
#include "firmware/src/io/i2c/peripherals/sensor_bno085/instance/impl/mapper.h"
#include "firmware/src/signaling/signaling.h"
#include "firmware/src/system/shared/config.h"

#include <array>
#include <optional>
#include <stddef.h>
#include <string_view>

namespace opendeck::firmware::io::i2c::sensor_bno085
{
    /**
     * @brief BNO085 IMU fusion sensor integration.
     */
    class SensorBno085 : public Peripheral
    {
        public:
        /**
         * @brief Constructs the BNO085 peripheral bound to the shared I2C backend.
         *
         * @param hwa Hardware abstraction used to communicate with the sensor.
         */
        explicit SensorBno085(Hwa& hwa, Database& database);

        /**
         * @brief Initializes BNO085 at one supported I2C address.
         *
         * @param address_index Index into the supported I2C address list.
         *
         * @return `true` if initialization completed.
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
         * @brief Returns supported BNO085 I2C addresses.
         *
         * @return Candidate 7-bit I2C addresses.
         */
        std::span<const uint8_t> i2c_addresses() const override;

        private:
        Hwa&                 _hwa;
        Database&            _database;
        Mapper               _mapper;
        bool                 _initialized                = false;
        size_t               _selected_i2c_address_index = 0;
        uint8_t              _control_sequence           = 0;
        RotationVectorFilter _rotation_vector_filter     = {};
        SensorVectorFilter   _gyroscope_filter           = {};
        SensorVectorFilter   _linear_accel_filter        = {};
        SensorVectorFilter   _gravity_filter             = {};

        /**
         * @brief Returns the selected BNO085 I2C address.
         *
         * @return Selected 7-bit I2C address.
         */
        uint8_t i2c_address() const;

        /**
         * @brief Sends the BNO085 SHTP soft reset packet.
         *
         * @return `true` if the packet was written.
         */
        bool soft_reset();

        /**
         * @brief Reads one startup SHTP packet header.
         *
         * @return `true` if a valid packet header was read.
         */
        bool read_startup_packet();

        /**
         * @brief Enables the sensor reports used for OSC-oriented IMU data.
         *
         * @return `true` if all commands were written.
         */
        bool enable_reports();

        /**
         * @brief Enables one BNO085 sensor report.
         *
         * @param report_id Sensor report ID.
         *
         * @return `true` if the command was written.
         */
        bool enable_report(uint8_t report_id);

        /**
         * @brief Returns whether a BNO085 data output is enabled.
         *
         * @param setting Output enable setting.
         *
         * @return `true` when the output should be published.
         */
        bool output_enabled(Setting setting) const;

        /**
         * @brief Reads and publishes one enabled sensor report when available.
         *
         * @return `true` if the sensor is still usable.
         */
        bool read_report();

        /**
         * @brief Parses and publishes one sensor report to OSC.
         *
         * @param report Report bytes.
         */
        void publish_report(std::span<const uint8_t> report);

        /**
         * @brief Publishes one mapped BNO085 result.
         *
         * @param result Mapper result to publish.
         */
        void publish_result(const Mapper::Result& result) const;

        /**
         * @brief Returns whether one report value changed since the last publish.
         *
         * @param report_id Sensor report ID.
         * @param values Raw report values.
         *
         * @return `true` when the report should be published.
         */
        bool should_publish(uint8_t report_id, const std::array<int16_t, 4>& values);

        /**
         * @brief Reads one BNO085 SysEx setting.
         *
         * @param section I2C SysEx section.
         * @param index Setting index.
         * @param value Output setting value.
         *
         * @return SysEx status when the section is handled.
         */
        std::optional<uint8_t> sys_config_get(sys::Config::Section::I2c section, size_t index, uint16_t& value);

        /**
         * @brief Writes one BNO085 SysEx setting.
         *
         * @param section I2C SysEx section.
         * @param index Setting index.
         * @param value New setting value.
         *
         * @return SysEx status when the section is handled.
         */
        std::optional<uint8_t> sys_config_set(sys::Config::Section::I2c section, size_t index, uint16_t value);

        /**
         * @brief Reads one signed little-endian 16-bit value from a report.
         *
         * @param report Report bytes.
         * @param offset First value byte.
         *
         * @return Decoded value.
         */
        static int16_t read_i16(std::span<const uint8_t> report, size_t offset);

        /**
         * @brief Writes a little-endian 32-bit value into a packet.
         *
         * @param packet Packet bytes.
         * @param offset First value byte.
         * @param value Value to write.
         */
        static void write_u32(std::span<uint8_t> packet, size_t offset, uint32_t value);

        /**
         * @brief Decodes an SHTP packet length from a four-byte header.
         *
         * @param header SHTP header bytes.
         *
         * @return Packet length with the continuation bit cleared.
         */
        static uint16_t packet_size(std::span<const uint8_t, SHTP_HEADER_SIZE> header);
    };
}    // namespace opendeck::firmware::io::i2c::sensor_bno085
