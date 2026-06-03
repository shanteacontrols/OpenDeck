/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <array>
#include <cstdint>

namespace opendeck::firmware::io::i2c::sensor_cap1188
{
    constexpr inline std::array<uint8_t, 5> I2C_ADDRESSES = {
        0x28,
        0x29,
        0x2A,
        0x2B,
        0x2C,
    };

    constexpr inline uint8_t CAP1188_PRODUCT_ID               = 0x50;
    constexpr inline uint8_t CAP1188_MANUFACTURER_ID          = 0x5D;
    constexpr inline uint8_t CAP1188_REGISTER_MAIN_CONTROL    = 0x00;
    constexpr inline uint8_t CAP1188_REGISTER_SENSOR_INPUT    = 0x03;
    constexpr inline uint8_t CAP1188_REGISTER_SENSITIVITY     = 0x1F;
    constexpr inline uint8_t CAP1188_REGISTER_LED_LINKING     = 0x72;
    constexpr inline uint8_t CAP1188_REGISTER_PRODUCT_ID      = 0xFD;
    constexpr inline uint8_t CAP1188_REGISTER_MANUFACTURER_ID = 0xFE;
    constexpr inline uint8_t CAP1188_REGISTER_REVISION        = 0xFF;
    constexpr inline uint8_t CAP1188_MAIN_CONTROL_INTERRUPT   = 0x01;
    constexpr inline uint8_t CAP1188_SENSOR_INPUT_COUNT       = 8;
    constexpr inline uint8_t CAP1188_LED_LINK_ALL_INPUTS      = 0xFF;
    constexpr inline uint8_t CAP1188_SENSITIVITY_MASK         = 0x70;
    constexpr inline uint8_t CAP1188_DELTA_SENSE_64X          = 0x10;
    constexpr inline uint8_t CAP1188_DELTA_SENSE_32X          = 0x20;
    constexpr inline uint8_t CAP1188_DELTA_SENSE_16X          = 0x30;
}    // namespace opendeck::firmware::io::i2c::sensor_cap1188
