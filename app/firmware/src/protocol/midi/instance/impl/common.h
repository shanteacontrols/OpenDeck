/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cstddef>
#include <cstdint>

namespace opendeck::firmware::protocol::midi
{
    /**
     * @brief Maximum number of UMP packets accumulated before a USB burst is flushed.
     */
    static constexpr size_t USB_UMP_BURST_PACKET_COUNT = 64;

    /**
     * @brief Selects how note-off events are encoded on output.
     */
    enum class NoteOffType : uint8_t
    {
        StandardNoteOff,
        NoteOnZeroVel,
    };
}    // namespace opendeck::firmware::protocol::midi
