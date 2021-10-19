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

#include "database/Database.h"
#include "util/messaging/Messaging.h"

#ifndef BUTTONS_SUPPORTED
#include "stub/Buttons.h"
#else

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
            // should return true if the value has been refreshed, false otherwise
            virtual bool state(size_t index, uint8_t& numberOfReadings, uint32_t& states) = 0;
        };

        class Filter
        {
            public:
            virtual bool isFiltered(size_t index, uint8_t& numberOfReadings, uint32_t& states) = 0;
        };

        Buttons(HWA&                     hwa,
                Filter&                  filter,
                Database&                database,
                Util::MessageDispatcher& dispatcher);

        void update(bool forceResend = false);
        bool state(size_t index);
        void reset(size_t index);

        private:
        struct buttonDescriptor_t
        {
            type_t                             type        = type_t::momentary;
            messageType_t                      messageType = messageType_t::note;
            Util::MessageDispatcher::message_t dispatchMessage;

            buttonDescriptor_t() = default;
        };

        void fillButtonDescriptor(size_t index, buttonDescriptor_t& descriptor);
        void processButton(size_t index, bool reading, buttonDescriptor_t& descriptor);
        void sendMessage(size_t index, bool state, buttonDescriptor_t& descriptor);
        void setState(size_t index, bool state);
        void setLatchingState(size_t index, bool state);
        bool latchingState(size_t index);

        HWA&                     _hwa;
        Filter&                  _filter;
        Database&                _database;
        Util::MessageDispatcher& _dispatcher;

        uint8_t _buttonPressed[(MAX_NUMBER_OF_BUTTONS + MAX_NUMBER_OF_ANALOG + MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS) / 8 + 1]     = {};
        uint8_t _lastLatchingState[(MAX_NUMBER_OF_BUTTONS + MAX_NUMBER_OF_ANALOG + MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS) / 8 + 1] = {};

        const MIDI::messageType_t _internalMsgToMIDIType[static_cast<uint8_t>(messageType_t::AMOUNT)] = {
            MIDI::messageType_t::noteOn,
            MIDI::messageType_t::programChange,
            MIDI::messageType_t::controlChange,
            MIDI::messageType_t::controlChange,
            MIDI::messageType_t::mmcStop,
            MIDI::messageType_t::mmcPlay,
            MIDI::messageType_t::mmcRecordStart,    // modified to stop when needed
            MIDI::messageType_t::mmcPause,
            MIDI::messageType_t::sysRealTimeClock,
            MIDI::messageType_t::sysRealTimeStart,
            MIDI::messageType_t::sysRealTimeContinue,
            MIDI::messageType_t::sysRealTimeStop,
            MIDI::messageType_t::sysRealTimeActiveSensing,
            MIDI::messageType_t::sysRealTimeSystemReset,
            MIDI::messageType_t::programChange,
            MIDI::messageType_t::programChange,
            MIDI::messageType_t::invalid,
            MIDI::messageType_t::invalid,
            MIDI::messageType_t::noteOn,
            MIDI::messageType_t::noteOn,
            MIDI::messageType_t::controlChange,
            MIDI::messageType_t::controlChange,
        };
    };
}    // namespace IO

#endif