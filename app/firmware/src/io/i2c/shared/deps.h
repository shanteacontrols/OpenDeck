/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <inttypes.h>
#include <stddef.h>

namespace opendeck::io::i2c
{
    /**
     * @brief Hardware abstraction for low-level I2C transactions.
     */
    class Hwa
    {
        public:
        virtual ~Hwa() = default;

        /**
         * @brief Initializes the I2C controller backend.
         *
         * @return `true` if initialization succeeded, otherwise `false`.
         */
        virtual bool init() = 0;

        /**
         * @brief Writes one buffer to the specified I2C address.
         *
         * @param address 7-bit I2C device address.
         * @param buffer Buffer containing bytes to transmit.
         * @param size Number of bytes to write.
         *
         * @return `true` if the transfer succeeded, otherwise `false`.
         */
        virtual bool write(uint8_t address, uint8_t* buffer, size_t size) = 0;

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
     * @brief Interface implemented by I2C peripherals managed by the subsystem.
     */
    class Peripheral
    {
        public:
        virtual ~Peripheral() = default;

        /**
         * @brief Initializes the peripheral.
         *
         * @return `true` if initialization succeeded, otherwise `false`.
         */
        virtual bool init() = 0;

        /**
         * @brief Performs one periodic peripheral update.
         */
        virtual void update() = 0;
    };
}    // namespace opendeck::io::i2c
