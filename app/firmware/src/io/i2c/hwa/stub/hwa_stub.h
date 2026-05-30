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
    class HwaStub : public HwaBase, public HwaPeripheral
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
         *
         * @return Always `false`.
         */
        bool write([[maybe_unused]] uint8_t address, [[maybe_unused]] std::span<const uint8_t> buffer) override
        {
            return false;
        }

        /**
         * @brief Rejects all write/read requests.
         *
         * @param address Target I2C address.
         * @param write_buffer Buffer that would be transmitted before the read.
         * @param read_buffer Buffer that would receive bytes from the device.
         *
         * @return Always `false`.
         */
        bool write_read([[maybe_unused]] uint8_t                  address,
                        [[maybe_unused]] std::span<const uint8_t> write_buffer,
                        [[maybe_unused]] std::span<uint8_t>       read_buffer) override
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
