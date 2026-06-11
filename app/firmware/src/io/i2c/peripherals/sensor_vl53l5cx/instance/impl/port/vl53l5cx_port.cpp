/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "vl53l5cx_class.h"

#include "zlibs/utils/misc/bit.h"

#include <algorithm>
#include <array>
#include <cstring>
#include <limits>
#include <span>
#include <zephyr/kernel.h>

#ifndef DEFAULT_I2C_BUFFER_LEN
#ifdef BUFFER_LENGTH
#define DEFAULT_I2C_BUFFER_LEN BUFFER_LENGTH
#else
#define DEFAULT_I2C_BUFFER_LEN 32
#endif
#endif

namespace
{
    constexpr uint8_t WORD_HIGH_BYTE_SHIFT  = zlibs::utils::misc::BYTE_BIT_COUNT;
    constexpr size_t  REGISTER_ADDRESS_SIZE = zlibs::utils::misc::WORD_SIZE_IN_BYTES;
}    // namespace

uint8_t VL53L5CX::RdByte(VL53L5CX_Platform* p_platform, uint16_t register_address, uint8_t* p_value)
{
    return RdMulti(p_platform, register_address, p_value, 1);
}

uint8_t VL53L5CX::WrByte(VL53L5CX_Platform* p_platform, uint16_t register_address, uint8_t value)
{
    return WrMulti(p_platform, register_address, &value, 1);
}

uint8_t VL53L5CX::WrMulti(VL53L5CX_Platform* p_platform, uint16_t register_address, uint8_t* p_values, uint32_t size)
{
    if ((p_platform == nullptr) || (p_platform->dev_i2c == nullptr))
    {
        return VL53L5CX_STATUS_ERROR;
    }

    const uint8_t address = static_cast<uint8_t>((p_platform->address >> 1U) & 0x7FU);
    uint32_t      offset  = 0;

    while (offset < size)
    {
        const size_t                                                        chunk_size = std::min<size_t>(DEFAULT_I2C_BUFFER_LEN, size - offset);
        std::array<uint8_t, DEFAULT_I2C_BUFFER_LEN + REGISTER_ADDRESS_SIZE> buffer     = {};

        buffer[0] = static_cast<uint8_t>((register_address + offset) >> WORD_HIGH_BYTE_SHIFT);
        buffer[1] = static_cast<uint8_t>((register_address + offset) & zlibs::utils::misc::BYTE_MASK);

        std::copy_n(p_values + offset, chunk_size, buffer.begin() + REGISTER_ADDRESS_SIZE);

        if (!p_platform->dev_i2c->write(address, std::span<const uint8_t>(buffer.data(), chunk_size + REGISTER_ADDRESS_SIZE)))
        {
            return VL53L5CX_STATUS_ERROR;
        }

        offset += chunk_size;
    }

    return VL53L5CX_STATUS_OK;
}

uint8_t VL53L5CX::RdMulti(VL53L5CX_Platform* p_platform, uint16_t register_address, uint8_t* p_values, uint32_t size)
{
    if ((p_platform == nullptr) || (p_platform->dev_i2c == nullptr))
    {
        return VL53L5CX_STATUS_ERROR;
    }

    const uint8_t address = static_cast<uint8_t>((p_platform->address >> 1U) & 0x7FU);

    const std::array<uint8_t, REGISTER_ADDRESS_SIZE> register_buffer = {
        static_cast<uint8_t>(register_address >> WORD_HIGH_BYTE_SHIFT),
        static_cast<uint8_t>(register_address & zlibs::utils::misc::BYTE_MASK),
    };

    if (!p_platform->dev_i2c->write_read(address, register_buffer, std::span<uint8_t>(p_values, size)))
    {
        return VL53L5CX_STATUS_ERROR;
    }

    return VL53L5CX_STATUS_OK;
}

void VL53L5CX::SwapBuffer(uint8_t* buffer, uint16_t size)
{
    for (uint16_t i = 0; i < size; i += sizeof(uint32_t))
    {
        const uint32_t value = (static_cast<uint32_t>(buffer[i]) << 24U) |
                               (static_cast<uint32_t>(buffer[i + 1U]) << 16U) |
                               (static_cast<uint32_t>(buffer[i + 2U]) << 8U) |
                               static_cast<uint32_t>(buffer[i + 3U]);

        std::memcpy(&buffer[i], &value, sizeof(value));
    }
}

uint8_t VL53L5CX::WaitMs([[maybe_unused]] VL53L5CX_Platform* p_platform, uint32_t time_ms)
{
    const auto delay_ms = std::min<uint32_t>(time_ms, static_cast<uint32_t>(std::numeric_limits<int32_t>::max()));

    k_msleep(static_cast<int32_t>(delay_ms));

    return VL53L5CX_STATUS_OK;
}
