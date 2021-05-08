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
    class Buttons
    {
        public:
        /// List of all possible button types.
        enum class type_t : uint8_t
        {
            momentary,    ///< Event on press and release.
            latching,     ///< Event between presses only.
            AMOUNT        ///< Total number of button types.
        };

        /// List of all possible MIDI messages buttons can send.
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
            //should return true if the value has been refreshed, false otherwise
            virtual bool state(size_t index, uint8_t& numberOfReadings, uint32_t& states) = 0;
        };

        class Filter
        {
            public:
            virtual bool isFiltered(size_t   index,
                                    bool     state,
                                    bool&    filteredState,
                                    uint32_t sampleTakenTime) = 0;

            virtual void reset(size_t index) = 0;
        };

        Buttons(HWA&           hwa,
                Filter&        filter,
                uint32_t       timeDiffTimeout,
                Database&      database,
                MIDI&          midi,
                IO::LEDs&      leds,
                Display&       display,
                ComponentInfo& cInfo)
            : _hwa(hwa)
            , _filter(filter)
            , TIME_DIFF_READOUT(timeDiffTimeout)
            , _database(database)
            , _midi(midi)
            , _leds(leds)
            , _display(display)
            , _cInfo(cInfo)
        {}

        void update(bool forceResend = false);
        void processButton(size_t index, bool newState);
        bool state(size_t index);
        void reset(size_t index);

        private:
        typedef struct
        {
            type_t        type;
            messageType_t midiMessage;
            uint8_t       midiID;
            uint8_t       midiChannel;
            uint8_t       velocity;
        } buttonDescriptor_t;

        void fillButtonDescriptor(size_t index, buttonDescriptor_t& descriptor);
        void sendMessage(size_t index, bool state, buttonDescriptor_t& descriptor);
        void setState(size_t index, bool state);
        void setLatchingState(size_t index, bool state);
        bool latchingState(size_t index);

        HWA&    _hwa;
        Filter& _filter;

        /// Time difference betweeen multiple button readouts in milliseconds.
        const uint32_t TIME_DIFF_READOUT;

        Database&      _database;
        MIDI&          _midi;
        IO::LEDs&      _leds;
        Display&       _display;
        ComponentInfo& _cInfo;

        /// Array holding current state for all buttons.
        uint8_t _buttonPressed[(MAX_NUMBER_OF_BUTTONS + MAX_NUMBER_OF_ANALOG + MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS) / 8 + 1] = {};

        /// Array holding last sent state for latching buttons only.
        uint8_t _lastLatchingState[(MAX_NUMBER_OF_BUTTONS + MAX_NUMBER_OF_ANALOG + MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS) / 8 + 1] = {};

        /// Array used for simpler building of transport control messages.
        /// Based on MIDI specification for transport control.
        uint8_t _mmcArray[6] = { 0xF0, 0x7F, 0x7F, 0x06, 0x00, 0xF7 };
    };
}    // namespace IO

#endif