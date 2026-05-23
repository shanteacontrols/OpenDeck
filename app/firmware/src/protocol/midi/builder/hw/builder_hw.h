/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/protocol/midi/instance/impl/midi.h"
#include "firmware/src/protocol/midi/hwa/hw/hwa_hw.h"
#include "firmware/src/protocol/midi/hwa/stub/hwa_stub.h"
#include "firmware/src/database/builder/builder.h"

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
#ifdef CONFIG_PROJECT_TARGET_SUPPORT_USB_MIDI
        using HwaUsbBuilder = HwaUsbHw;
#else
        using HwaUsbBuilder = HwaUsbStub;
#endif

#ifdef CONFIG_PROJECT_TARGET_SUPPORT_DIN_MIDI
        using HwaSerialBuilder = HwaSerialHw;
#else
        using HwaSerialBuilder = HwaSerialStub;
#endif

#ifdef CONFIG_PROJECT_TARGET_SUPPORT_BLE
        using HwaBleBuilder = HwaBleHw;
#else
        using HwaBleBuilder = HwaBleStub;
#endif

        HwaUsbBuilder    _hwa_usb;
        HwaSerialBuilder _hwa_serial;
        HwaBleBuilder    _hwa_ble;
        UsbMidi          _usb    = UsbMidi(_hwa_usb);
        SerialMidi       _serial = SerialMidi(_hwa_serial);
        BleMidi          _ble    = BleMidi(_hwa_ble);
        Database         _database;
        Midi             _instance;
    };
}    // namespace opendeck::protocol::midi
