/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/io/common/shared/common.h"
#include "firmware/src/io/outputs/drivers/count.h"
#include "firmware/src/io/touchscreen/drivers/count.h"

namespace opendeck::io::outputs
{
    /**
     * @brief Flattened collection of OUTPUT-capable outputs.
     */
    class Collection : public io::common::BaseCollection<OPENDECK_OUTPUT_LOGICAL_COUNT,
                                                         OPENDECK_TOUCHSCREEN_COMPONENT_COUNT>
    {
        public:
        /**
         * @brief Prevents instantiation of this compile-time OUTPUT collection descriptor.
         */
        Collection() = delete;
    };

    /**
     * @brief Group indices used with `Collection`.
     */
    enum
    {
        GroupDigitalOutputs,
        GroupTouchscreenComponents
    };

    /**
     * @brief Identifies one component of an RGB OUTPUT.
     */
    enum class RgbComponent : uint8_t
    {
        R,
        G,
        B
    };

    /**
     * @brief Logical OUTPUT colors supported by the system.
     */
    enum class Color : uint8_t
    {
        Off,
        Red,
        Green,
        Yellow,
        Blue,
        Magenta,
        Cyan,
        White,
        Count
    };

    /**
     * @brief Global OUTPUT settings stored in the database.
     */
    enum class Setting : uint8_t
    {
        BlinkWithMidiClock,
        Unused,
        UseStartupAnimation,
        UseMidiProgramOffset,
        Count
    };

    /**
     * @brief Selects which signal source controls an OUTPUT.
     */
    enum class ControlType : uint8_t
    {
        MidiInNoteSingleVal,
        LocalNoteSingleVal,
        MidiInCcSingleVal,
        LocalCcSingleVal,
        PcSingleVal,
        Preset,
        MidiInNoteMultiVal,
        LocalNoteMultiVal,
        MidiInCcMultiVal,
        LocalCcMultiVal,
        Static,
        Count
    };

    /**
     * @brief Supported OUTPUT blink rates.
     */
    enum class BlinkSpeed : uint8_t
    {
        Ms1000,
        Ms500,
        Ms250,
        NoBlink
    };

    /**
     * @brief Selects the timing source used for OUTPUT blinking.
     */
    enum class BlinkType : uint8_t
    {
        Timer,
        MidiClock
    };

    /**
     * @brief Supported logical OUTPUT brightness levels.
     */
    enum class Brightness : uint8_t
    {
        Off,
        Level25,
        Level50,
        Level75,
        Level100
    };
}    // namespace opendeck::io::outputs
