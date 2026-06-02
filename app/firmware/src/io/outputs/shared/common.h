/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/io/shared/common.h"

namespace opendeck::firmware::io::outputs
{
    /**
     * @brief Flattened collection of physical outputs and touchscreen components.
     */
    class Collection : public io::common::BaseCollection<CONFIG_PROJECT_TARGET_OUTPUT_LOGICAL_COUNT,
                                                         CONFIG_PROJECT_TARGET_TOUCHSCREEN_COMPONENT_COUNT>
    {
        public:
        /**
         * @brief Prevents instantiation of this compile-time output collection descriptor.
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
     * @brief Global output settings stored in the database.
     */
    enum class Setting : uint8_t
    {
        PulseWithMidiClock,
        Unused,
        UseStartupAnimation,
        UseMidiProgramOffset,
        Count
    };

    /**
     * @brief Selects which signal source controls an output.
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
     * @brief Supported output pulse rates.
     */
    enum class PulseSpeed : uint8_t
    {
        Ms1000,
        Ms500,
        Ms250,
        NoPulse
    };

    /**
     * @brief Selects the timing source used for output pulsing.
     */
    enum class PulseMode : uint8_t
    {
        Timer,
        MidiClock
    };

    /** @brief Minimum logical output level, expressed as percent. */
    constexpr uint8_t OUTPUT_LEVEL_MIN = 0;

    /** @brief Maximum logical output level, expressed as percent. */
    constexpr uint8_t OUTPUT_LEVEL_MAX = 100;
}    // namespace opendeck::firmware::io::outputs
