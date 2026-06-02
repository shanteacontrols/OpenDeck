/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/io/i2c/shared/deps.h"

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/i2c.h>

namespace opendeck::firmware::io::i2c
{
    /**
     * @brief Hardware-backed I2C backend that proxies to the Zephyr I2C driver.
     */
    class HwaHw : public HwaBase, public HwaPeripheral
    {
        public:
        HwaHw() = default;

        /**
         * @brief Initializes the selected I2C controller by checking device readiness.
         *
         * @return `true` if the controller is ready, otherwise `false`.
         */
        bool init() override
        {
            return device_is_ready(_i2c_device);
        }

        /**
         * @brief Writes one buffer to the requested I2C address.
         *
         * @param address 7-bit I2C device address.
         * @param buffer Buffer containing bytes to transmit.
         *
         * @return `true` if the transfer succeeded, otherwise `false`.
         */
        bool write(uint8_t address, std::span<const uint8_t> buffer) override
        {
            return i2c_write(_i2c_device, buffer.data(), buffer.size(), address) == 0;
        }

        /**
         * @brief Reads one buffer from the requested I2C address.
         *
         * @param address 7-bit I2C device address.
         * @param buffer Buffer that receives bytes read from the device.
         *
         * @return `true` if the transfer succeeded, otherwise `false`.
         */
        bool read(uint8_t address, std::span<uint8_t> buffer) override
        {
            return i2c_read(_i2c_device, buffer.data(), buffer.size(), address) == 0;
        }

        /**
         * @brief Writes one buffer and then reads one buffer from the requested I2C address.
         *
         * @param address 7-bit I2C device address.
         * @param write_buffer Buffer containing bytes to transmit before the read.
         * @param read_buffer Buffer that receives bytes read from the device.
         *
         * @return `true` if the transfer succeeded, otherwise `false`.
         */
        bool write_read(uint8_t address, std::span<const uint8_t> write_buffer, std::span<uint8_t> read_buffer) override
        {
            return i2c_write_read(_i2c_device, address, write_buffer.data(), write_buffer.size(), read_buffer.data(), read_buffer.size()) == 0;
        }

        /**
         * @brief Returns whether a device acknowledges the specified I2C address.
         *
         * @param address 7-bit I2C device address.
         *
         * @return `true` if the device is available, otherwise `false`.
         */
        bool device_available(uint8_t address) override
        {
            return i2c_write(_i2c_device, nullptr, 0, address) == 0;
        }

        private:
        static_assert(DT_NODE_EXISTS(DT_NODELABEL(opendeck_i2c)), "OpenDeck I2C node must exist.");
        static_assert(DT_NODE_HAS_PROP(DT_NODELABEL(opendeck_i2c), bus), "OpenDeck I2C node must define a bus phandle.");

        const device* const _i2c_device = DEVICE_DT_GET(DT_PHANDLE(DT_NODELABEL(opendeck_i2c), bus));
    };
}    // namespace opendeck::firmware::io::i2c
