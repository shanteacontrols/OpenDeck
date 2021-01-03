/*

Copyright 2015-2021 Igor Petrovic

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

#ifndef BUTTONS_SUPPORTED
#include "Stub.h"
#else

#include "database/Database.h"
#include "midi/src/MIDI.h"
#include "io/leds/LEDs.h"
#include "io/display/Display.h"
#include "io/common/CInfo.h"

namespace IO
{
    ///
    /// \brief Button handling.
    /// \defgroup interfaceButtons Buttons
    /// \ingroup interfaceDigitalIn
    /// @{

    class Buttons
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
            multiValIncResetNote,
            multiValIncDecNote,
            multiValIncResetCC,
            multiValIncDecCC,
            AMOUNT
        };

        class HWA
        {
            public:
            virtual bool state(size_t index) = 0;
        };

        class Filter
        {
            public:
            virtual bool isFiltered(size_t index, bool value, bool& filteredValue) = 0;
            virtual void reset(size_t index)                                       = 0;
        };

        Buttons(HWA&           hwa,
                Filter&        filter,
                Database&      database,
                MIDI&          midi,
                IO::LEDs&      leds,
                Display&       display,
                ComponentInfo& cInfo)
            : hwa(hwa)
            , filter(filter)
            , database(database)
            , midi(midi)
            , leds(leds)
            , display(display)
            , cInfo(cInfo)
        {}

        void update();
        void processButton(uint8_t buttonID, bool state);
        bool getButtonState(uint8_t buttonID);
        void reset(uint8_t buttonID);

        private:
        typedef struct
        {
            messageType_t messageType;
            uint8_t       note;
            uint8_t       channel;
            uint8_t       velocity;
        } buttonMessageDescriptor_t;

        void sendMessage(uint8_t buttonID, bool state, buttonMessageDescriptor_t& descriptor);
        void setButtonState(uint8_t buttonID, uint8_t state);
        void setLatchingState(uint8_t buttonID, uint8_t state);
        bool getLatchingState(uint8_t buttonID);

        HWA&           hwa;
        Filter&        filter;
        Database&      database;
        MIDI&          midi;
        IO::LEDs&      leds;
        Display&       display;
        ComponentInfo& cInfo;

        ///
        /// \brief Array holding current state for all buttons.
        ///
        uint8_t buttonPressed[(MAX_NUMBER_OF_BUTTONS + MAX_NUMBER_OF_ANALOG + MAX_NUMBER_OF_TOUCHSCREEN_BUTTONS) / 8 + 1] = {};

        ///
        /// \brief Array holding last sent state for latching buttons only.
        ///
        uint8_t lastLatchingState[(MAX_NUMBER_OF_BUTTONS + MAX_NUMBER_OF_ANALOG + MAX_NUMBER_OF_TOUCHSCREEN_BUTTONS) / 8 + 1] = {};

        ///
        /// \brief Array used for simpler building of transport control messages.
        /// Based on MIDI specification for transport control.
        ///
        uint8_t mmcArray[6] = { 0xF0, 0x7F, 0x7F, 0x06, 0x00, 0xF7 };
    };

    /// @}
}    // namespace IO

#endif