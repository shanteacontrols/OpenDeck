/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "vl53l4cx_class.h"

#include "zlibs/utils/misc/bit.h"

#include <algorithm>
#include <array>
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
    constexpr uint8_t WORD_HIGH_BYTE_SHIFT     = zlibs::utils::misc::BYTE_BIT_COUNT;
    constexpr uint8_t DWORD_HIGHEST_BYTE_SHIFT = zlibs::utils::misc::BYTE_BIT_COUNT * (zlibs::utils::misc::DWORD_SIZE_IN_BYTES - 1);
    constexpr uint8_t DWORD_HIGH_BYTE_SHIFT    = zlibs::utils::misc::BYTE_BIT_COUNT * zlibs::utils::misc::WORD_SIZE_IN_BYTES;
    constexpr uint8_t DWORD_LOW_BYTE_SHIFT     = zlibs::utils::misc::BYTE_BIT_COUNT;
    constexpr size_t  REGISTER_ADDRESS_SIZE    = zlibs::utils::misc::WORD_SIZE_IN_BYTES;
}    // namespace

VL53L4CX_Error VL53L4CX::VL53L4CX_WriteMulti(VL53L4CX_DEV dev, uint16_t index, uint8_t* data, uint32_t count)
{
    return VL53L4CX_I2CWrite(dev->I2cDevAddr, index, data, static_cast<uint16_t>(count));
}

VL53L4CX_Error VL53L4CX::VL53L4CX_ReadMulti(VL53L4CX_DEV dev, uint16_t index, uint8_t* data, uint32_t count)
{
    return VL53L4CX_I2CRead(dev->I2cDevAddr, index, data, static_cast<uint16_t>(count));
}

VL53L4CX_Error VL53L4CX::VL53L4CX_WrByte(VL53L4CX_DEV dev, uint16_t index, uint8_t data)
{
    return VL53L4CX_I2CWrite(dev->I2cDevAddr, index, &data, 1);
}

VL53L4CX_Error VL53L4CX::VL53L4CX_WrWord(VL53L4CX_DEV dev, uint16_t index, uint16_t data)
{
    std::array<uint8_t, zlibs::utils::misc::WORD_SIZE_IN_BYTES> buffer = {
        static_cast<uint8_t>(data >> WORD_HIGH_BYTE_SHIFT),
        static_cast<uint8_t>(data & zlibs::utils::misc::BYTE_MASK),
    };

    return VL53L4CX_I2CWrite(dev->I2cDevAddr, index, buffer.data(), buffer.size());
}

VL53L4CX_Error VL53L4CX::VL53L4CX_WrDWord(VL53L4CX_DEV dev, uint16_t index, uint32_t data)
{
    std::array<uint8_t, zlibs::utils::misc::DWORD_SIZE_IN_BYTES> buffer = {
        static_cast<uint8_t>((data >> DWORD_HIGHEST_BYTE_SHIFT) & zlibs::utils::misc::BYTE_MASK),
        static_cast<uint8_t>((data >> DWORD_HIGH_BYTE_SHIFT) & zlibs::utils::misc::BYTE_MASK),
        static_cast<uint8_t>((data >> DWORD_LOW_BYTE_SHIFT) & zlibs::utils::misc::BYTE_MASK),
        static_cast<uint8_t>(data & zlibs::utils::misc::BYTE_MASK),
    };

    return VL53L4CX_I2CWrite(dev->I2cDevAddr, index, buffer.data(), buffer.size());
}

VL53L4CX_Error VL53L4CX::VL53L4CX_RdByte(VL53L4CX_DEV dev, uint16_t index, uint8_t* data)
{
    return VL53L4CX_I2CRead(dev->I2cDevAddr, index, data, 1);
}

VL53L4CX_Error VL53L4CX::VL53L4CX_RdWord(VL53L4CX_DEV dev, uint16_t index, uint16_t* data)
{
    std::array<uint8_t, zlibs::utils::misc::WORD_SIZE_IN_BYTES> buffer = {};
    const auto                                                  status = VL53L4CX_I2CRead(dev->I2cDevAddr, index, buffer.data(), buffer.size());

    if (status == VL53L4CX_ERROR_NONE)
    {
        *data = static_cast<uint16_t>((static_cast<uint16_t>(buffer[0]) << WORD_HIGH_BYTE_SHIFT) | buffer[1]);
    }

    return status;
}

VL53L4CX_Error VL53L4CX::VL53L4CX_RdDWord(VL53L4CX_DEV dev, uint16_t index, uint32_t* data)
{
    std::array<uint8_t, zlibs::utils::misc::DWORD_SIZE_IN_BYTES> buffer = {};
    const auto                                                   status = VL53L4CX_I2CRead(dev->I2cDevAddr, index, buffer.data(), buffer.size());

    if (status == VL53L4CX_ERROR_NONE)
    {
        *data = (static_cast<uint32_t>(buffer[0]) << DWORD_HIGHEST_BYTE_SHIFT) |
                (static_cast<uint32_t>(buffer[1]) << DWORD_HIGH_BYTE_SHIFT) |
                (static_cast<uint32_t>(buffer[2]) << DWORD_LOW_BYTE_SHIFT) |
                static_cast<uint32_t>(buffer[3]);
    }

    return status;
}

VL53L4CX_Error VL53L4CX::VL53L4CX_UpdateByte(VL53L4CX_DEV dev, uint16_t index, uint8_t and_data, uint8_t or_data)
{
    uint8_t buffer = 0;
    auto    status = VL53L4CX_I2CRead(dev->I2cDevAddr, index, &buffer, 1);

    if (status == VL53L4CX_ERROR_NONE)
    {
        buffer = (buffer & and_data) | or_data;
        status = VL53L4CX_I2CWrite(dev->I2cDevAddr, index, &buffer, 1);
    }

    return status;
}

VL53L4CX_Error VL53L4CX::VL53L4CX_I2CWrite(uint8_t device_address, uint16_t register_address, uint8_t* data, uint16_t size)
{
    if (dev_i2c == nullptr)
    {
        return VL53L4CX_ERROR_CONTROL_INTERFACE;
    }

    const uint8_t address = static_cast<uint8_t>((device_address >> 1) & 0x7F);
    uint32_t      offset  = 0;

    while (offset < size)
    {
        const size_t                                                        chunk_size = std::min<size_t>(DEFAULT_I2C_BUFFER_LEN, size - offset);
        std::array<uint8_t, DEFAULT_I2C_BUFFER_LEN + REGISTER_ADDRESS_SIZE> buffer     = {};

        buffer[0] = static_cast<uint8_t>((register_address + offset) >> WORD_HIGH_BYTE_SHIFT);
        buffer[1] = static_cast<uint8_t>((register_address + offset) & zlibs::utils::misc::BYTE_MASK);

        std::copy_n(data + offset, chunk_size, buffer.begin() + REGISTER_ADDRESS_SIZE);

        if (!dev_i2c->write(address, std::span<const uint8_t>(buffer.data(), chunk_size + REGISTER_ADDRESS_SIZE)))
        {
            return VL53L4CX_ERROR_CONTROL_INTERFACE;
        }

        offset += chunk_size;
    }

    return VL53L4CX_ERROR_NONE;
}

VL53L4CX_Error VL53L4CX::VL53L4CX_I2CRead(uint8_t device_address, uint16_t register_address, uint8_t* data, uint16_t size)
{
    if (dev_i2c == nullptr)
    {
        return VL53L4CX_ERROR_CONTROL_INTERFACE;
    }

    const uint8_t address = static_cast<uint8_t>((device_address >> 1) & 0x7F);

    const std::array<uint8_t, zlibs::utils::misc::WORD_SIZE_IN_BYTES> register_buffer = {
        static_cast<uint8_t>(register_address >> WORD_HIGH_BYTE_SHIFT),
        static_cast<uint8_t>(register_address & zlibs::utils::misc::BYTE_MASK),
    };

    if (!dev_i2c->write_read(address, register_buffer, std::span<uint8_t>(data, size)))
    {
        return VL53L4CX_ERROR_CONTROL_INTERFACE;
    }

    return VL53L4CX_ERROR_NONE;
}

VL53L4CX_Error VL53L4CX::VL53L4CX_GetTickCount(uint32_t* tick_count_ms)
{
    *tick_count_ms = k_uptime_get_32();
    return VL53L4CX_ERROR_NONE;
}

VL53L4CX_Error VL53L4CX::VL53L4CX_WaitUs([[maybe_unused]] VL53L4CX_Dev_t* dev, int32_t wait_us)
{
    k_sleep(K_USEC(wait_us));
    return VL53L4CX_ERROR_NONE;
}

VL53L4CX_Error VL53L4CX::VL53L4CX_WaitMs([[maybe_unused]] VL53L4CX_Dev_t* dev, int32_t wait_ms)
{
    k_msleep(wait_ms);
    return VL53L4CX_ERROR_NONE;
}

VL53L4CX_Error VL53L4CX::VL53L4CX_WaitValueMaskEx(VL53L4CX_Dev_t* dev,
                                                  uint32_t        timeout_ms,
                                                  uint16_t        index,
                                                  uint8_t         value,
                                                  uint8_t         mask,
                                                  uint32_t        poll_delay_ms)
{
    uint32_t start_time_ms   = 0;
    uint32_t current_time_ms = 0;
    uint8_t  byte_value      = 0;

    VL53L4CX_GetTickCount(&start_time_ms);

    do
    {
        const auto status = VL53L4CX_RdByte(dev, index, &byte_value);

        if (status != VL53L4CX_ERROR_NONE)
        {
            return status;
        }

        if ((byte_value & mask) == value)
        {
            return VL53L4CX_ERROR_NONE;
        }

        if (poll_delay_ms > 0)
        {
            VL53L4CX_WaitMs(dev, static_cast<int32_t>(poll_delay_ms));
        }

        VL53L4CX_GetTickCount(&current_time_ms);
    } while ((current_time_ms - start_time_ms) < timeout_ms);

    return VL53L4CX_ERROR_TIME_OUT;
}
