/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "io/common/common.h"
#include "io/digital/drivers/count.h"

namespace io::encoders
{
    /**
     * @brief Flattened collection of encoder inputs.
     */
    class Collection : public io::common::BaseCollection<OPENDECK_ENCODER_COUNT>
    {
        public:
        /**
         * @brief Prevents instantiation of this compile-time encoder collection descriptor.
         */
        Collection() = delete;
    };

    /**
     * @brief Selects the action emitted by an encoder step.
     */
    enum class Type : uint8_t
    {
        ControlChange7fh01h,
        ControlChange3fh41h,
        ProgramChange,
        ControlChange,
        PresetChange,
        PitchBend,
        Nrpn7Bit,
        Nrpn14Bit,
        ControlChange14Bit,
        ControlChange41h01h,
        BpmChange,
        SingleNoteVariableVal,
        SingleNoteFixedValBothDir,
        SingleNoteFixedValOneDir0OtherDir,
        TwoNoteFixedValBothDir,
        Count
    };

    /**
     * @brief Logical direction decoded from an encoder sample.
     */
    enum class Position : uint8_t
    {
        Stopped,
        Ccw,
        Cw,
    };

    /**
     * @brief Supported encoder acceleration profiles.
     */
    enum class Acceleration : uint8_t
    {
        Disabled,
        Slow,
        Medium,
        Fast,
        Count
    };
}    // namespace io::encoders

#undef OPENDECK_ENCODER_COUNT
