/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cstdint>

namespace opendeck::firmware::io::outputs
{
    /**
     * @brief Selects the timing source used for output pulsing.
     */
    enum class PulseMode : uint8_t
    {
        Timer,
        MidiClock
    };
}    // namespace opendeck::firmware::io::outputs
