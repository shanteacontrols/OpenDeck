/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/io/i2c/shared/deps.h"

namespace opendeck::io::i2c
{
    /**
     * @brief Stub I2C backend that reports no hardware support.
     */
    class HwaStub : public Hwa
    {
        public:
        HwaStub() = default;

        /**
         * @brief Reports that the stub backend cannot initialize hardware.
         *
         * @return Always `false`.
         */
        bool init() override
        {
            return false;
        }

        /**
         * @brief Rejects all write requests.
         *
         * @param address Target I2C address.
         * @param buffer Buffer that would be transmitted.
         * @param size Number of bytes that would be written.
         *
         * @return Always `false`.
         */
        bool write([[maybe_unused]] uint8_t  address,
                   [[maybe_unused]] uint8_t* buffer,
                   [[maybe_unused]] size_t   size) override
        {
            return false;
        }

        /**
         * @brief Reports that no I2C devices are available through the stub backend.
         *
         * @param address Target I2C address.
         *
         * @return Always `false`.
         */
        bool device_available([[maybe_unused]] uint8_t address) override
        {
            return false;
        }
    };
}    // namespace opendeck::io::i2c
