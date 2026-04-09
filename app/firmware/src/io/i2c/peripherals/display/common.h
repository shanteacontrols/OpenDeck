/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <inttypes.h>

namespace io::i2c::display
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
        Enable,
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
}    // namespace io::i2c::display
