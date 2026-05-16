/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/protocol/midi/midi.h"
#include "firmware/src/protocol/midi/hwa/hw/hwa_hw.h"
#include "firmware/src/database/builder.h"

namespace opendeck::protocol::midi
{
    /**
     * @brief Convenience builder that wires the hardware MIDI subsystem.
     */
    class Builder
    {
        public:
        /**
         * @brief Constructs the MIDI builder around the shared database instance.
         *
         * @param database Database administrator used for configuration access.
         */
        explicit Builder(database::Admin& database)
            : _database(database)
            , _instance(_usb, _serial, _ble, _database)
        {}

        /**
         * @brief Returns the constructed MIDI subsystem instance.
         *
         * @return Hardware-backed MIDI subsystem instance.
         */
        Midi& instance()
        {
            return _instance;
        }

        private:
        HwaUsbHw    _hwa_usb;
        HwaSerialHw _hwa_serial;
        HwaBleHw    _hwa_ble;
        UsbMidi     _usb    = UsbMidi(_hwa_usb);
        SerialMidi  _serial = SerialMidi(_hwa_serial);
        BleMidi     _ble    = BleMidi(_hwa_ble);
        Database    _database;
        Midi        _instance;
    };
}    // namespace opendeck::protocol::midi
