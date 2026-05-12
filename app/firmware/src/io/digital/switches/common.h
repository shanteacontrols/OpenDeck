/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "io/common/common.h"
#include "io/analog/drivers/count.h"
#include "io/digital/drivers/count.h"
#include "io/touchscreen/count.h"

namespace opendeck::io::switches
{
    /** @brief Debounce interval applied to digital switch inputs in milliseconds. */
    constexpr inline uint32_t DEBOUNCE_TIME_MS = 8;

    /**
     * @brief Flattened collection of all switch-capable inputs.
     */
    class Collection : public io::common::BaseCollection<OPENDECK_SWITCH_LOGICAL_COUNT,
                                                         OPENDECK_ANALOG_LOGICAL_COUNT,
                                                         OPENDECK_TOUCHSCREEN_COMPONENT_COUNT>
    {
        public:
        /**
         * @brief Prevents instantiation of this compile-time switch collection descriptor.
         */
        Collection() = delete;
    };

    /**
     * @brief Group indices used with `Collection`.
     */
    enum
    {
        GroupDigitalInputs,
        GroupAnalogInputs,
        GroupTouchscreenComponents
    };

    /**
     * @brief Selects how a switch reports state changes.
     */
    enum class Type : uint8_t
    {
        Momentary,
        Latching,
        Count
    };

    /**
     * @brief Selects the action emitted when a switch changes state.
     */
    enum class MessageType : uint8_t
    {
        Note,
        ProgramChange,
        ControlChange,
        ControlChangeReset,
        MmcStop,
        MmcPlay,
        MmcRecord,
        MmcPause,
        RealTimeClock,
        RealTimeStart,
        RealTimeContinue,
        RealTimeStop,
        RealTimeActiveSensing,
        RealTimeSystemReset,
        ProgramChangeInc,
        ProgramChangeDec,
        None,
        PresetChange,
        MultiValIncResetNote,
        MultiValIncDecNote,
        MultiValIncResetCc,
        MultiValIncDecCc,
        NoteOffOnly,
        ControlChange0Only,
        Reserved,
        ProgramChangeOffsetInc,
        ProgramChangeOffsetDec,
        BpmInc,
        BpmDec,
        MmcPlayStop,
        Count
    };
}    // namespace opendeck::io::switches
