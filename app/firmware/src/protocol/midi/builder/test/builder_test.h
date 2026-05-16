/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/protocol/midi/midi.h"
#include "firmware/src/protocol/midi/hwa/test/hwa_test.h"
#include "firmware/src/database/builder/test/builder_test.h"

namespace opendeck::protocol::midi
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
            , _instance(_usb, _serial, _ble, _database)
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
        UsbMidi       _usb    = UsbMidi(_hwaUsb);
        SerialMidi    _serial = SerialMidi(_hwaSerial);
        BleMidi       _ble    = BleMidi(_hwaBle);
        Database      _database;
        Midi          _instance;
    };
}    // namespace opendeck::protocol::midi
