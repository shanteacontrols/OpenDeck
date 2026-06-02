/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/protocol/base.h"

namespace opendeck::firmware::protocol::midi
{
    /**
     * @brief Stub MIDI protocol used when no MIDI transport is enabled.
     */
    class Midi : public protocol::Base
    {
        public:
        Midi()           = default;
        ~Midi() override = default;

        bool init() override
        {
            return false;
        }

        bool deinit() override
        {
            return true;
        }
    };
}    // namespace opendeck::firmware::protocol::midi
