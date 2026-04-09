/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "midi.h"
#include "hwa_test.h"
#include "database/builder_test.h"

namespace protocol::midi
{
    /**
     * @brief Test builder that wires the MIDI subsystem to test transport backends.
     */
    class Builder
    {
        public:
        /**
         * @brief Constructs the MIDI test builder around the shared database instance.
         *
         * @param database Database administrator used for configuration access.
         */
        Builder(database::Admin& database)
            : _database(database)
            , _instance(_hwaUsb, _hwaSerial, _hwaBle, _database)
        {}

        /**
         * @brief Returns the constructed MIDI subsystem instance.
         *
         * @return Test-backed MIDI subsystem instance.
         */
        Midi& instance()
        {
            return _instance;
        }

        HwaUsbTest    _hwaUsb;
        HwaSerialTest _hwaSerial;
        HwaBleTest    _hwaBle;
        Database      _database;
        Midi          _instance;
    };
}    // namespace protocol::midi
