/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <array>
#include <cstdint>

namespace opendeck::firmware::io::i2c::sensor_vl53l4cx
{
    constexpr inline std::array<uint8_t, 1> I2C_ADDRESSES = {
        0x29,
    };

    constexpr inline uint16_t VL53L4CX_REGISTER_SOFT_RESET = 0x0000;
    constexpr inline uint16_t VL53L4CX_REGISTER_MODEL_ID   = 0x010F;
    constexpr inline uint16_t VL53L4CX_EXPECTED_SENSOR_ID  = 0xEBAA;
}    // namespace opendeck::firmware::io::i2c::sensor_vl53l4cx
