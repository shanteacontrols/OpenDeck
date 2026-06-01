/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <inttypes.h>
#include <stddef.h>
#include <span>
#include <string_view>

namespace opendeck::io::i2c
{
    /**
     * @brief Hardware abstraction used by the I2C subsystem itself.
     */
    class HwaBase
    {
        public:
        virtual ~HwaBase() = default;

        /**
         * @brief Initializes the I2C controller backend.
         *
         * @return `true` if initialization succeeded, otherwise `false`.
         */
        virtual bool init() = 0;

        /**
         * @brief Returns whether a device acknowledges the specified address.
         *
         * @param address 7-bit I2C device address.
         *
         * @return `true` if the device is available, otherwise `false`.
         */
        virtual bool device_available(uint8_t address) = 0;
    };

    /**
     * @brief Hardware abstraction used by I2C peripherals for transfers.
     */
    class HwaPeripheral
    {
        public:
        virtual ~HwaPeripheral() = default;

        /**
         * @brief Writes one buffer to the specified I2C address.
         *
         * @param address 7-bit I2C device address.
         * @param buffer Buffer containing bytes to transmit.
         *
         * @return `true` if the transfer succeeded, otherwise `false`.
         */
        virtual bool write(uint8_t address, std::span<const uint8_t> buffer) = 0;

        /**
         * @brief Reads one buffer from the specified I2C address.
         *
         * @param address 7-bit I2C device address.
         * @param buffer Buffer that receives bytes read from the device.
         *
         * @return `true` if the transfer succeeded, otherwise `false`.
         */
        virtual bool read(uint8_t address, std::span<uint8_t> buffer) = 0;

        /**
         * @brief Writes one buffer and then reads one buffer from the specified I2C address.
         *
         * @param address 7-bit I2C device address.
         * @param write_buffer Buffer containing bytes to transmit before the read.
         * @param read_buffer Buffer that receives bytes read from the device.
         *
         * @return `true` if the transfer succeeded, otherwise `false`.
         */
        virtual bool write_read(uint8_t address, std::span<const uint8_t> write_buffer, std::span<uint8_t> read_buffer) = 0;
    };

    /**
     * @brief Interface implemented by I2C peripherals managed by the subsystem.
     */
    class Peripheral
    {
        public:
        virtual ~Peripheral() = default;

        /**
         * @brief Initializes the peripheral using one of its advertised addresses.
         *
         * @param address_index Index into the array returned by i2c_addresses().
         *
         * @return `true` if initialization succeeded, otherwise `false`.
         */
        virtual bool init(size_t address_index) = 0;

        /**
         * @brief Deinitializes the peripheral runtime state.
         *
         * @return `true` if deinitialization succeeded, otherwise `false`.
         */
        virtual bool deinit() = 0;

        /**
         * @brief Performs one periodic peripheral update.
         *
         * @return `true` if the peripheral is still usable, otherwise `false`.
         */
        virtual bool update() = 0;

        /**
         * @brief Returns the peripheral name used in diagnostics.
         *
         * @return Static peripheral name.
         */
        virtual constexpr std::string_view name() const = 0;

        /**
         * @brief Returns I2C addresses that identify this peripheral.
         *
         * @return Candidate 7-bit I2C addresses.
         */
        virtual std::span<const uint8_t> i2c_addresses() const = 0;
    };
}    // namespace opendeck::io::i2c
