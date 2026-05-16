/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/io/i2c/shared/deps.h"

namespace opendeck::io::i2c
{
    /**
     * @brief Test I2C backend that accepts writes without touching hardware.
     */
    class HwaTest : public Hwa
    {
        public:
        HwaTest() = default;

        /**
         * @brief Initializes the test backend.
         *
         * @return Always `true`.
         */
        bool init() override
        {
            return true;
        }

        /**
         * @brief Accepts all write requests in tests.
         *
         * @param address Target I2C address.
         * @param buffer Buffer that would be transmitted.
         * @param size Number of bytes that would be written.
         *
         * @return Always `true`.
         */
        bool write(uint8_t address, uint8_t* buffer, size_t size) override
        {
            return true;
        }

        /**
         * @brief Reports that no test I2C devices are discoverable by default.
         *
         * @param address Target I2C address.
         *
         * @return Always `false`.
         */
        bool device_available(uint8_t address) override
        {
            return false;
        }
    };
}    // namespace opendeck::io::i2c
