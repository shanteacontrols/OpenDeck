/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/io/i2c/deps.h"

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/i2c.h>

namespace opendeck::io::i2c
{
    /**
     * @brief Hardware-backed I2C backend that proxies to the Zephyr I2C driver.
     */
    class HwaHw : public Hwa
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
         * @param size Number of bytes to write.
         *
         * @return `true` if the transfer succeeded, otherwise `false`.
         */
        bool write(uint8_t address, uint8_t* buffer, size_t size) override
        {
            return i2c_write(_i2c_device, buffer, size, address) == 0;
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
        static_assert(DT_NODE_EXISTS(DT_NODELABEL(opendeck_display)), "OpenDeck display node must exist.");
        static_assert(DT_NODE_HAS_PROP(DT_NODELABEL(opendeck_display), i2c), "OpenDeck display node must define an i2c phandle.");

        const device* const _i2c_device = DEVICE_DT_GET(DT_PHANDLE(DT_NODELABEL(opendeck_display), i2c));
    };
}    // namespace opendeck::io::i2c
