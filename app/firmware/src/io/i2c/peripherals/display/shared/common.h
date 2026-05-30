/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <array>
#include <inttypes.h>

namespace opendeck::io::i2c::display
{
    /**
     * @brief Identifies configurable display settings stored in the database.
     */
    enum class Setting : uint8_t
    {
        DeviceInfoMsg,
        Controller,
        Resolution,
        EventTime,
        MidiNotesAlternate,
        OctaveNormalization,
        Reserved,
        Count
    };

    /**
     * @brief Identifies the supported display controller backends.
     */
    enum class DisplayController : uint8_t
    {
        Invalid,
        Ssd1306,
        Count
    };

    /**
     * @brief Identifies the supported display resolutions.
     */
    enum DisplayResolution : uint8_t
    {
        Invalid,
        Res128x64,
        Res128x32,
        Count
    };

    constexpr inline std::array<uint32_t, static_cast<uint8_t>(Setting::Count)> DISPLAY_DEFAULTS = {
        0,
        static_cast<uint8_t>(DisplayController::Ssd1306),
        static_cast<uint8_t>(DisplayResolution::Res128x64),
        0,
        0,
        0,
        0,
    };

    constexpr inline std::array<uint8_t, 2> I2C_ADDRESSES = {
        0x3C,
        0x3D,
    };
}    // namespace opendeck::io::i2c::display
