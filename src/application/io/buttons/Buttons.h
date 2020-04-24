/*

Copyright 2015-2020 Igor Petrovic

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*/

#pragma once

#include "database/Database.h"
#include "midi/src/MIDI.h"
#include "io/leds/LEDs.h"
#ifdef DISPLAY_SUPPORTED
#include "io/display/Display.h"
#endif
#include "io/common/Common.h"
#include "io/common/CInfo.h"

namespace IO
{
    ///
    /// \brief Button handling.
    /// \defgroup interfaceButtons Buttons
    /// \ingroup interfaceDigitalIn
    /// @{

    class Buttons : public Common
    {
        public:
        ///
        /// \brief List of all possible button types.
        ///
        enum class type_t : uint8_t
        {
            momentary,    ///< Event on press and release.
            latching,     ///< Event between presses only.
            AMOUNT        ///< Total number of button types.
        };

        ///
        /// \brief List of all possible MIDI messages buttons can send.
        ///
        enum class messageType_t : uint8_t
        {
            note,
            programChange,
            controlChange,
            controlChangeReset,
            mmcStop,
            mmcPlay,
            mmcRecord,
            mmcPause,
            realTimeClock,
            realTimeStart,
            realTimeContinue,
            realTimeStop,
            realTimeActiveSensing,
            realTimeSystemReset,
            programChangeInc,
            programChangeDec,
            none,
            presetOpenDeck,
            customHook,
            AMOUNT
        };

#ifdef DISPLAY_SUPPORTED
        Buttons(Database& database, MIDI& midi, IO::LEDs& leds, Display& display, ComponentInfo& cInfo)
#else
        Buttons(Database& database, MIDI& midi, IO::LEDs& leds, ComponentInfo& cInfo)
#endif
            : database(database)
            , midi(midi)
            , leds(leds)
#ifdef DISPLAY_SUPPORTED
            , display(display)
#endif
            , cInfo(cInfo)
        {}

        void update();
        void processButton(uint8_t buttonID, bool state);
        bool getButtonState(uint8_t buttonID);
        void reset(uint8_t buttonID);

        private:
        void sendMessage(uint8_t buttonID, bool state, messageType_t buttonMessage = messageType_t::AMOUNT);
        void setButtonState(uint8_t buttonID, uint8_t state);
        void setLatchingState(uint8_t buttonID, uint8_t state);
        bool getLatchingState(uint8_t buttonID);
        bool buttonDebounced(uint8_t buttonID, bool state);
        void customHook(uint8_t buttonID, bool state);

        Database& database;
        MIDI&     midi;
        IO::LEDs& leds;
#ifdef DISPLAY_SUPPORTED
        Display& display;
#endif
        ComponentInfo& cInfo;

        ///
        /// \brief Array holding debounce count for all buttons to avoid incorrect state detection.
        ///
        uint8_t buttonDebounceCounter[MAX_NUMBER_OF_BUTTONS + MAX_NUMBER_OF_ANALOG + MAX_TOUCHSCREEN_BUTTONS] = {};

        ///
        /// \brief Array holding current state for all buttons.
        ///
        uint8_t buttonPressed[(MAX_NUMBER_OF_BUTTONS + MAX_NUMBER_OF_ANALOG + MAX_TOUCHSCREEN_BUTTONS) / 8 + 1] = {};

        ///
        /// \brief Array holding last sent state for latching buttons only.
        ///
        uint8_t lastLatchingState[(MAX_NUMBER_OF_BUTTONS + MAX_NUMBER_OF_ANALOG + MAX_TOUCHSCREEN_BUTTONS) / 8 + 1] = {};

        ///
        /// \brief Array used for simpler building of transport control messages.
        /// Based on MIDI specification for transport control.
        ///
        uint8_t mmcArray[6] = { 0xF0, 0x7F, 0x7F, 0x06, 0x00, 0xF7 };
    };

    /// @}
}    // namespace IO