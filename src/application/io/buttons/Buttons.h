/*

Copyright 2015-2022 Igor Petrovic

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

#include <array>
#include "database/Database.h"
#include "protocol/midi/MIDI.h"
#include "global/MIDIProgram.h"
#include "system/Config.h"
#include "io/IOBase.h"

using namespace Protocol;

#if defined(HW_SUPPORT_DIGITAL_INPUTS) || defined(HW_SUPPORT_TOUCHSCREEN) || defined(HW_SUPPORT_ADC)
#define BUTTONS_SUPPORTED

namespace IO
{
    class Buttons : public IO::Base
    {
        public:
        class Collection : public Common::BaseCollection<HW_SUPPORTED_NR_OF_DIGITAL_INPUTS,
                                                         HW_SUPPORTED_NR_OF_ANALOG_INPUTS,
                                                         HW_SUPPORTED_NR_OF_TOUCHSCREEN_COMPONENTS>
        {
            public:
            Collection() = delete;
        };

        enum
        {
            GROUP_DIGITAL_INPUTS,
            GROUP_ANALOG_INPUTS,
            GROUP_TOUCHSCREEN_COMPONENTS
        };

        enum class type_t : uint8_t
        {
            MOMENTARY,    ///< Event on press and release.
            LATCHING,     ///< Event between presses only.
            AMOUNT        ///< Total number of button types.
        };

        enum class messageType_t : uint8_t
        {
            NOTE,
            PROGRAM_CHANGE,
            CONTROL_CHANGE,
            CONTROL_CHANGE_RESET,
            MMC_STOP,
            MMC_PLAY,
            MMC_RECORD,
            MMC_PAUSE,
            REAL_TIME_CLOCK,
            REAL_TIME_START,
            REAL_TIME_CONTINUE,
            REAL_TIME_STOP,
            REAL_TIME_ACTIVE_SENSING,
            REAL_TIME_SYSTEM_RESET,
            PROGRAM_CHANGE_INC,
            PROGRAM_CHANGE_DEC,
            NONE,
            PRESET_CHANGE,
            MULTI_VAL_INC_RESET_NOTE,
            MULTI_VAL_INC_DEC_NOTE,
            MULTI_VAL_INC_RESET_CC,
            MULTI_VAL_INC_DEC_CC,
            NOTE_OFF_ONLY,
            CONTROL_CHANGE0_ONLY,
            DMX,
            PROGRAM_CHANGE_OFFSET_INC,
            PROGRAM_CHANGE_OFFSET_DEC,
            AMOUNT
        };

        class HWA
        {
            public:
            virtual ~HWA() = default;

            // should return true if the value has been refreshed, false otherwise
            virtual bool   state(size_t index, uint8_t& numberOfReadings, uint32_t& states) = 0;
            virtual size_t buttonToEncoderIndex(size_t index)                               = 0;
        };

        class Filter
        {
            public:
            virtual ~Filter() = default;

            virtual bool isFiltered(size_t index, uint8_t& numberOfReadings, uint32_t& states) = 0;
        };

        using Database = Database::User<Database::Config::Section::button_t,
                                        Database::Config::Section::encoder_t>;

        Buttons(HWA&      hwa,
                Filter&   filter,
                Database& database);

        bool   init() override;
        void   updateSingle(size_t index, bool forceRefresh = false) override;
        void   updateAll(bool forceRefresh = false) override;
        size_t maxComponentUpdateIndex() override;
        void   reset(size_t index);

        private:
        struct buttonDescriptor_t
        {
            type_t             type        = type_t::MOMENTARY;
            messageType_t      messageType = messageType_t::NOTE;
            Messaging::event_t event;

            buttonDescriptor_t() = default;
        };

        using ValueIncDecMIDI7Bit = Util::IncDec<uint8_t, 0, ::MIDI::MIDI_7BIT_VALUE_MAX>;

        bool                   state(size_t index);
        bool                   state(size_t index, uint8_t& numberOfReadings, uint32_t& states);
        void                   fillButtonDescriptor(size_t index, buttonDescriptor_t& descriptor);
        void                   processButton(size_t index, bool reading, buttonDescriptor_t& descriptor);
        void                   sendMessage(size_t index, bool state, buttonDescriptor_t& descriptor);
        void                   setState(size_t index, bool state);
        void                   setLatchingState(size_t index, bool state);
        bool                   latchingState(size_t index);
        std::optional<uint8_t> sysConfigGet(System::Config::Section::button_t section, size_t index, uint16_t& value);
        std::optional<uint8_t> sysConfigSet(System::Config::Section::button_t section, size_t index, uint16_t value);

        HWA&               _hwa;
        Filter&            _filter;
        Buttons::Database& _database;

        uint8_t _buttonPressed[Collection::SIZE() / 8 + 1]     = {};
        uint8_t _lastLatchingState[Collection::SIZE() / 8 + 1] = {};
        uint8_t _incDecValue[Collection::SIZE()]               = {};

        static constexpr std::array<MIDI::messageType_t, static_cast<uint8_t>(messageType_t::AMOUNT)> INTERNAL_MSG_TO_MIDI_TYPE = {
            MIDI::messageType_t::NOTE_ON,                         // NOTE
            MIDI::messageType_t::PROGRAM_CHANGE,                  // PROGRAM_CHANGE
            MIDI::messageType_t::CONTROL_CHANGE,                  // CONTROL_CHANGE
            MIDI::messageType_t::CONTROL_CHANGE,                  // CONTROL_CHANGE_RESET
            MIDI::messageType_t::MMC_STOP,                        // MMC_STOP
            MIDI::messageType_t::MMC_PLAY,                        // MMC_PLAY
            MIDI::messageType_t::MMC_RECORD_START,                // MMC_RECORD - modified to stop when needed
            MIDI::messageType_t::MMC_PAUSE,                       // MMC_PAUSE
            MIDI::messageType_t::SYS_REAL_TIME_CLOCK,             // REAL_TIME_CLOCK
            MIDI::messageType_t::SYS_REAL_TIME_START,             // REAL_TIME_START
            MIDI::messageType_t::SYS_REAL_TIME_CONTINUE,          // REAL_TIME_CONTINUE
            MIDI::messageType_t::SYS_REAL_TIME_STOP,              // REAL_TIME_STOP
            MIDI::messageType_t::SYS_REAL_TIME_ACTIVE_SENSING,    // REAL_TIME_ACTIVE_SENSING
            MIDI::messageType_t::SYS_REAL_TIME_SYSTEM_RESET,      // REAL_TIME_SYSTEM_RESET
            MIDI::messageType_t::PROGRAM_CHANGE,                  // PROGRAM_CHANGE_INC
            MIDI::messageType_t::PROGRAM_CHANGE,                  // PROGRAM_CHANGE_DEC
            MIDI::messageType_t::INVALID,                         // NONE
            MIDI::messageType_t::INVALID,                         // PRESET_CHANGE
            MIDI::messageType_t::NOTE_ON,                         // MULTI_VAL_INC_RESET_NOTE
            MIDI::messageType_t::NOTE_ON,                         // MULTI_VAL_INC_DEC_NOTE
            MIDI::messageType_t::CONTROL_CHANGE,                  // MULTI_VAL_INC_RESET_CC
            MIDI::messageType_t::CONTROL_CHANGE,                  // MULTI_VAL_INC_DEC_CC
            MIDI::messageType_t::NOTE_ON,                         // NOTE_OFF_ONLY
            MIDI::messageType_t::CONTROL_CHANGE,                  // CONTROL_CHANGE0_ONLY
            MIDI::messageType_t::INVALID,                         // DMX
            MIDI::messageType_t::INVALID,                         // PROGRAM_CHANGE_OFFSET_INC
            MIDI::messageType_t::INVALID,                         // PROGRAM_CHANGE_OFFSET_DEC
        };
    };
}    // namespace IO

#else
#include "stub/Buttons.h"
#endif