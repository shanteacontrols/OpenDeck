/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/io/i2c/instance/impl/deps.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <vector>

namespace opendeck::firmware::io::i2c
{
    /**
     * @brief Test I2C backend with optional register-map behavior.
     */
    class HwaTest : public HwaBase, public HwaPeripheral
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
            initialized = true;
            return true;
        }

        /**
         * @brief Accepts all write requests in tests.
         *
         * @param address Target I2C address.
         * @param buffer Buffer that would be transmitted.
         *
         * @return Always `true`.
         */
        bool write(uint8_t address, std::span<const uint8_t> buffer) override
        {
            write_addresses.push_back(address);
            last_write.assign(buffer.begin(), buffer.end());
            written_bytes = buffer.size();

            if (!connected || !address_allowed(address))
            {
                return false;
            }

            if (register_write_mode)
            {
                if (buffer.size() < 2U)
                {
                    return false;
                }

                registers[buffer[0]] = buffer[1];
            }

            return write_succeeds;
        }

        /**
         * @brief Returns queued raw read data in tests.
         *
         * @param address Target I2C address.
         * @param buffer Buffer that receives test data.
         *
         * @return Configurable read status.
         */
        bool read(uint8_t address, std::span<uint8_t> buffer) override
        {
            read_addresses.push_back(address);

            if (!connected || !address_allowed(address))
            {
                return false;
            }

            for (size_t i = 0; i < buffer.size(); i++)
            {
                buffer[i] = i < read_data.size() ? read_data[i] : 0;
            }

            return read_succeeds;
        }

        /**
         * @brief Accepts all write/read requests in tests.
         *
         * @param address Target I2C address.
         * @param write_buffer Buffer that would be transmitted before the read.
         * @param read_buffer Buffer that receives test data.
         *
         * @return Always `true`.
         */
        bool write_read(uint8_t address, std::span<const uint8_t> write_buffer, std::span<uint8_t> read_buffer) override
        {
            write_read_addresses.push_back(address);
            last_write_read.assign(write_buffer.begin(), write_buffer.end());

            if (!connected || !address_allowed(address))
            {
                return false;
            }

            if (!register_read_mode)
            {
                for (auto& byte : read_buffer)
                {
                    byte = 0;
                }

                return write_read_succeeds;
            }

            if (write_buffer.size() != 1U)
            {
                return false;
            }

            const uint8_t reg = write_buffer[0];

            if (device_id_enabled && (reg == device_id_register))
            {
                read_buffer[0] = device_id;
                return write_read_succeeds;
            }

            if (fifo_enabled && (reg == fifo_register))
            {
                for (size_t i = 0; i < read_buffer.size(); i++)
                {
                    read_buffer[i] = fifo[i];
                }

                if (clear_register_after_fifo_read)
                {
                    registers[fifo_clear_register] = 0;
                    clear_register_after_fifo_read = false;
                }

                return write_read_succeeds;
            }

            for (size_t i = 0; i < read_buffer.size(); i++)
            {
                read_buffer[i] = registers[reg + i];
            }

            return write_read_succeeds;
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
            return connected &&
                   (std::find(available_addresses.begin(), available_addresses.end(), address) != available_addresses.end());
        }

        void set_available_addresses(std::initializer_list<uint8_t> addresses)
        {
            available_addresses.assign(addresses.begin(), addresses.end());
        }

        void set_u8(uint8_t reg, uint8_t value)
        {
            registers[reg] = value;
        }

        void set_u16(uint8_t reg, uint16_t value)
        {
            registers[reg]     = static_cast<uint8_t>(value & 0xFF);
            registers[reg + 1] = static_cast<uint8_t>(value >> 8);
        }

        void set_device_id_register(uint8_t reg, uint8_t value)
        {
            device_id_enabled  = true;
            device_id_register = reg;
            device_id          = value;
        }

        void set_gesture_fifo(uint8_t                                       fifo_reg,
                              uint8_t                                       level_reg,
                              uint8_t                                       status_reg,
                              uint8_t                                       valid_status,
                              std::initializer_list<std::array<uint8_t, 4>> samples)
        {
            size_t i = 0;

            for (const auto& sample : samples)
            {
                fifo[(i * 4U) + 0U] = sample[0];
                fifo[(i * 4U) + 1U] = sample[1];
                fifo[(i * 4U) + 2U] = sample[2];
                fifo[(i * 4U) + 3U] = sample[3];
                i++;
            }

            fifo_enabled                   = true;
            fifo_register                  = fifo_reg;
            fifo_clear_register            = status_reg;
            clear_register_after_fifo_read = true;

            registers[level_reg]  = static_cast<uint8_t>(i);
            registers[status_reg] = valid_status;
        }

        bool address_allowed(uint8_t address) const
        {
            return !require_available_address_for_transfers ||
                   (connected &&
                    (std::find(available_addresses.begin(), available_addresses.end(), address) != available_addresses.end()));
        }

        bool                     initialized                             = false;
        bool                     connected                               = true;
        bool                     write_succeeds                          = true;
        bool                     read_succeeds                           = true;
        bool                     write_read_succeeds                     = true;
        bool                     register_write_mode                     = false;
        bool                     register_read_mode                      = false;
        bool                     require_available_address_for_transfers = false;
        size_t                   written_bytes                           = 0;
        std::vector<uint8_t>     available_addresses                     = {};
        std::vector<uint8_t>     write_addresses                         = {};
        std::vector<uint8_t>     read_addresses                          = {};
        std::vector<uint8_t>     write_read_addresses                    = {};
        std::vector<uint8_t>     last_write                              = {};
        std::vector<uint8_t>     read_data                               = {};
        std::vector<uint8_t>     last_write_read                         = {};
        std::array<uint8_t, 256> registers                               = {};
        std::array<uint8_t, 128> fifo                                    = {};
        uint8_t                  device_id                               = 0;
        uint8_t                  device_id_register                      = 0;
        bool                     device_id_enabled                       = false;
        uint8_t                  fifo_register                           = 0;
        uint8_t                  fifo_clear_register                     = 0;
        bool                     fifo_enabled                            = false;
        bool                     clear_register_after_fifo_read          = false;
    };
}    // namespace opendeck::firmware::io::i2c
