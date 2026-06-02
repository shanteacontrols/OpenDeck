/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/protocol/midi/instance/stub/midi_stub.h"
#include "firmware/src/protocol/midi/hwa/stub/hwa_stub.h"
#include "firmware/src/database/builder/builder.h"

namespace opendeck::firmware::protocol::midi
{
    /**
     * @brief Stub builder that exposes a disabled MIDI protocol.
     */
    class Builder
    {
        public:
        /**
         * @brief Constructs the disabled MIDI builder.
         *
         * @param database Unused database administrator retained for builder compatibility.
         */
        explicit Builder([[maybe_unused]] database::Admin& database)
        {}

        /**
         * @brief Returns the disabled MIDI protocol instance.
         *
         * @return Stub MIDI protocol instance.
         */
        Midi& instance()
        {
            return _instance;
        }

        HwaUsbStub    _hwaUsb;
        HwaSerialStub _hwaSerial;
        HwaBleStub    _hwaBle;

        private:
        Midi _instance;
    };
}    // namespace opendeck::firmware::protocol::midi
