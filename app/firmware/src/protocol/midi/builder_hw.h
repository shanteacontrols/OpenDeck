/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "midi.h"
#include "hwa_hw.h"
#include "database/builder.h"

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
            , _instance(_hwa_usb, _hwa_serial, _hwa_ble, _database)
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
        Database    _database;
        Midi        _instance;
    };
}    // namespace opendeck::protocol::midi
