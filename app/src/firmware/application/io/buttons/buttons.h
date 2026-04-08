/*

Copyright Igor Petrovic

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

#include "deps.h"
#include "application/protocol/midi/midi.h"
#include "application/global/midi_program.h"
#include "application/system/config.h"
#include "application/io/base.h"

#include <optional>

namespace io::buttons
{
    class Buttons : public io::Base
    {
        public:
        Buttons(Hwa&      hwa,
                Filter&   filter,
                Database& database);

        bool   init() override;
        void   updateSingle(size_t index, bool forceRefresh = false) override;
        void   updateAll(bool forceRefresh = false) override;
        size_t maxComponentUpdateIndex() override;
        void   reset(size_t index);

        private:
        struct Descriptor
        {
            type_t           type        = type_t::MOMENTARY;
            messageType_t    messageType = messageType_t::NOTE;
            messaging::Event event       = {};
        };

        using ValueIncDecMIDI7Bit = util::IncDec<uint8_t, 0, protocol::midi::MAX_VALUE_7BIT>;

        static constexpr std::array<protocol::midi::messageType_t, static_cast<uint8_t>(messageType_t::AMOUNT)> INTERNAL_MSG_TO_MIDI_TYPE = {
            protocol::midi::messageType_t::NOTE_ON,                         // NOTE
            protocol::midi::messageType_t::PROGRAM_CHANGE,                  // PROGRAM_CHANGE
            protocol::midi::messageType_t::CONTROL_CHANGE,                  // CONTROL_CHANGE
            protocol::midi::messageType_t::CONTROL_CHANGE,                  // CONTROL_CHANGE_RESET
            protocol::midi::messageType_t::MMC_STOP,                        // MMC_STOP
            protocol::midi::messageType_t::MMC_PLAY,                        // MMC_PLAY
            protocol::midi::messageType_t::MMC_RECORD_START,                // MMC_RECORD - modified to stop when needed
            protocol::midi::messageType_t::MMC_PAUSE,                       // MMC_PAUSE
            protocol::midi::messageType_t::SYS_REAL_TIME_CLOCK,             // REAL_TIME_CLOCK
            protocol::midi::messageType_t::SYS_REAL_TIME_START,             // REAL_TIME_START
            protocol::midi::messageType_t::SYS_REAL_TIME_CONTINUE,          // REAL_TIME_CONTINUE
            protocol::midi::messageType_t::SYS_REAL_TIME_STOP,              // REAL_TIME_STOP
            protocol::midi::messageType_t::SYS_REAL_TIME_ACTIVE_SENSING,    // REAL_TIME_ACTIVE_SENSING
            protocol::midi::messageType_t::SYS_REAL_TIME_SYSTEM_RESET,      // REAL_TIME_SYSTEM_RESET
            protocol::midi::messageType_t::PROGRAM_CHANGE,                  // PROGRAM_CHANGE_INC
            protocol::midi::messageType_t::PROGRAM_CHANGE,                  // PROGRAM_CHANGE_DEC
            protocol::midi::messageType_t::INVALID,                         // NONE
            protocol::midi::messageType_t::INVALID,                         // PRESET_CHANGE
            protocol::midi::messageType_t::NOTE_ON,                         // MULTI_VAL_INC_RESET_NOTE
            protocol::midi::messageType_t::NOTE_ON,                         // MULTI_VAL_INC_DEC_NOTE
            protocol::midi::messageType_t::CONTROL_CHANGE,                  // MULTI_VAL_INC_RESET_CC
            protocol::midi::messageType_t::CONTROL_CHANGE,                  // MULTI_VAL_INC_DEC_CC
            protocol::midi::messageType_t::NOTE_ON,                         // NOTE_OFF_ONLY
            protocol::midi::messageType_t::CONTROL_CHANGE,                  // CONTROL_CHANGE0_ONLY
            protocol::midi::messageType_t::INVALID,                         // RESERVED
            protocol::midi::messageType_t::INVALID,                         // PROGRAM_CHANGE_OFFSET_INC
            protocol::midi::messageType_t::INVALID,                         // PROGRAM_CHANGE_OFFSET_DEC
            protocol::midi::messageType_t::INVALID,                         // BPM_INC
            protocol::midi::messageType_t::INVALID,                         // BPM_DEC
            protocol::midi::messageType_t::MMC_PLAY,                        // MMC_PLAY_STOP - modified to stop when needed
        };

        Hwa&      _hwa;
        Filter&   _filter;
        Database& _database;
        uint8_t   _buttonPressed[Collection::SIZE() / 8 + 1]     = {};
        uint8_t   _lastLatchingState[Collection::SIZE() / 8 + 1] = {};
        uint8_t   _incDecValue[Collection::SIZE()]               = {};

        bool                   state(size_t index);
        bool                   state(size_t index, uint8_t& numberOfReadings, uint16_t& states);
        void                   fillDescriptor(size_t index, Descriptor& descriptor);
        void                   processButton(size_t index, bool reading, Descriptor& descriptor);
        void                   sendMessage(size_t index, bool state, Descriptor& descriptor);
        void                   setState(size_t index, bool state);
        void                   setLatchingState(size_t index, bool state);
        bool                   latchingState(size_t index);
        std::optional<uint8_t> sysConfigGet(sys::Config::Section::button_t section, size_t index, uint16_t& value);
        std::optional<uint8_t> sysConfigSet(sys::Config::Section::button_t section, size_t index, uint16_t value);
    };
}    // namespace io::buttons