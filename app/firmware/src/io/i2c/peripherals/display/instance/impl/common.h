/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <array>
#include <cstdint>

namespace opendeck::firmware::io::i2c::display
{
    constexpr inline std::array<uint8_t, 2> I2C_ADDRESSES = {
        0x3C,
        0x3D,
    };
}    // namespace opendeck::firmware::io::i2c::display
