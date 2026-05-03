/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "io/common/common.h"
#include "io/leds/drivers/count.h"
#include "io/touchscreen/count.h"

namespace opendeck::io::leds
{
    /**
     * @brief Flattened collection of LED-capable outputs.
     */
    class Collection : public io::common::BaseCollection<OPENDECK_LED_OUTPUT_LOGICAL_COUNT,
                                                         OPENDECK_TOUCHSCREEN_COMPONENT_COUNT>
    {
        public:
        /**
         * @brief Prevents instantiation of this compile-time LED collection descriptor.
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
     * @brief Identifies one component of an RGB LED.
     */
    enum class RgbComponent : uint8_t
    {
        R,
        G,
        B
    };

    /**
     * @brief Logical LED colors supported by the system.
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
     * @brief Global LED settings stored in the database.
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
     * @brief Selects which signal source controls an LED.
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
     * @brief Supported LED blink rates.
     */
    enum class BlinkSpeed : uint8_t
    {
        Ms1000,
        Ms500,
        Ms250,
        NoBlink
    };

    /**
     * @brief Selects the timing source used for LED blinking.
     */
    enum class BlinkType : uint8_t
    {
        Timer,
        MidiClock
    };

    /**
     * @brief Supported logical LED brightness levels.
     */
    enum class Brightness : uint8_t
    {
        Off,
        Level25,
        Level50,
        Level75,
        Level100
    };
}    // namespace opendeck::io::leds
