/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <array>
#include <cstdint>

namespace opendeck::firmware::io::i2c::sensor_vl53l5cx
{
    constexpr inline std::array<uint8_t, 1> I2C_ADDRESSES = {
        0x29,
    };

    constexpr inline uint16_t VL53L5CX_REGISTER_PAGE_SELECT = 0x7FFF;
    constexpr inline uint16_t VL53L5CX_REGISTER_DEVICE_ID   = 0x0000;
    constexpr inline uint16_t VL53L5CX_REGISTER_REVISION_ID = 0x0001;

    constexpr inline uint8_t VL53L5CX_IDENTITY_PAGE = 0x00;
    constexpr inline uint8_t VL53L5CX_DEFAULT_PAGE  = 0x02;

    constexpr inline uint8_t VL53L5CX_EXPECTED_DEVICE_ID   = 0xF0;
    constexpr inline uint8_t VL53L5CX_EXPECTED_REVISION_ID = 0x02;
}    // namespace opendeck::firmware::io::i2c::sensor_vl53l5cx
