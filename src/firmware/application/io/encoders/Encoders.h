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
#include "io/IOBase.h"
#include "messaging/Messaging.h"
#include "global/MIDIProgram.h"
#include "system/Config.h"
#include "io/IOBase.h"
#include "protocol/midi/MIDI.h"

using namespace protocol;

#if defined(PROJECT_TARGET_SUPPORT_DIGITAL_INPUTS) && (PROJECT_TARGET_SUPPORTED_NR_OF_DIGITAL_INPUTS > 1)
#define ENCODERS_SUPPORTED

namespace io
{
    class Encoders : public io::Base
    {
        public:
        class Collection : public common::BaseCollection<PROJECT_TARGET_SUPPORTED_NR_OF_DIGITAL_INPUTS / 2>
        {
            public:
            Collection() = delete;
        };

        enum class type_t : uint8_t
        {
            CONTROL_CHANGE_7FH01H,
            CONTROL_CHANGE_3FH41H,
            PROGRAM_CHANGE,
            CONTROL_CHANGE,
            PRESET_CHANGE,
            PITCH_BEND,
            NRPN_7BIT,
            NRPN_14BIT,
            CONTROL_CHANGE_14BIT,
            RESERVED,
            AMOUNT
        };

        enum class position_t : uint8_t
        {
            STOPPED,
            CCW,
            CW,
        };

        enum class acceleration_t : uint8_t
        {
            DISABLED,
            SLOW,
            MEDIUM,
            FAST,
            AMOUNT
        };

        class HWA
        {
            public:
            virtual ~HWA() = default;

            // should return true if the value has been refreshed, false otherwise
            virtual bool state(size_t index, uint8_t& numberOfReadings, uint32_t& states) = 0;
        };

        class Filter
        {
            public:
            virtual ~Filter() = default;

            virtual bool isFiltered(size_t                    index,
                                    io::Encoders::position_t  position,
                                    io::Encoders::position_t& filteredPosition,
                                    uint32_t                  sampleTakenTime) = 0;

            virtual void     reset(size_t index)            = 0;
            virtual uint32_t lastMovementTime(size_t index) = 0;
        };

        using Database = database::User<database::Config::Section::encoder_t,
                                        database::Config::Section::global_t>;

        Encoders(HWA&      hwa,
                 Filter&   filter,
                 Database& database,
                 uint32_t  timeDiffTimeout = 1);

        bool   init() override;
        void   updateSingle(size_t index, bool forceRefresh = false) override;
        void   updateAll(bool forceRefresh = false) override;
        size_t maxComponentUpdateIndex() override;
        void   reset(size_t index);

        private:
        struct encoderDescriptor_t
        {
            type_t             type = type_t::CONTROL_CHANGE_7FH01H;
            messaging::event_t event;

            encoderDescriptor_t() = default;
        };

        void                   fillEncoderDescriptor(size_t index, encoderDescriptor_t& descriptor);
        position_t             read(size_t index, uint8_t pairState);
        void                   processReading(size_t index, uint8_t pairValue, uint32_t sampleTime);
        void                   sendMessage(size_t index, position_t encoderState, encoderDescriptor_t& descriptor);
        void                   setValue(size_t index, uint16_t value);
        std::optional<uint8_t> sysConfigGet(sys::Config::Section::encoder_t section, size_t index, uint16_t& value);
        std::optional<uint8_t> sysConfigSet(sys::Config::Section::encoder_t section, size_t index, uint16_t value);

        HWA&      _hwa;
        Filter&   _filter;
        Database& _database;

        using ValueIncDecMIDI7Bit  = util::IncDec<uint8_t, 0, ::MIDI::MIDI_7BIT_VALUE_MAX>;
        using ValueIncDecMIDI14Bit = util::IncDec<uint16_t, 0, ::MIDI::MIDI_14BIT_VALUE_MAX>;

        /// Time difference betweeen multiple encoder readouts in milliseconds.
        const uint32_t TIME_DIFF_READOUT;

        /// Time threshold in milliseconds between two encoder steps used to detect fast movement.
        static constexpr uint32_t ENCODERS_SPEED_TIMEOUT = 140;

        /// Holds current value for all encoders.
        int16_t _value[Collection::SIZE()] = { 0 };

        /// Array holding current speed (in steps) for all encoders.
        uint8_t _encoderSpeed[Collection::SIZE()] = {};

        /// Array holding last two readings from encoder pins.
        uint8_t _encoderData[Collection::SIZE()] = {};

        /// Array holding current amount of pulses for all encoders.
        int8_t _encoderPulses[Collection::SIZE()] = {};

        /// Lookup table used to convert encoder reading to pulses.
        static constexpr int8_t ENCODER_LOOK_UP_TABLE[16] = {
            0,     // 0000
            1,     // 0001
            -1,    // 0010
            0,     // 0011
            -1,    // 0100
            0,     // 0101
            0,     // 0110
            1,     // 0111
            1,     // 1000
            0,     // 1001
            0,     // 1010
            -1,    // 1011
            0,     // 1100
            -1,    // 1101
            1,     // 1110
            0      // 1111
        };

        /// Used to achieve linear encoder acceleration on fast movement.
        /// Every time fast movement is detected, amount of steps is increased by this value.
        /// Used only in CC/Pitch bend/NRPN modes. In Pitch bend/NRPN modes, this value is multiplied
        /// by 4 due to a larger value range.
        static constexpr uint8_t ENCODER_SPEED_CHANGE[static_cast<uint8_t>(io::Encoders::acceleration_t::AMOUNT)] = {
            0,    // acceleration disabled
            1,
            2,
            3
        };

        /// Value by which value is increased during acceleration.
        static constexpr uint8_t ENCODER_ACC_STEP_INC[static_cast<uint8_t>(io::Encoders::acceleration_t::AMOUNT)] = {
            0,    // acceleration disabled
            5,
            10,
            100
        };

        static constexpr uint8_t VAL_CONTROL_CHANGE_7FH01H[3] = {
            0,      // STOPPED
            127,    // CCW
            1       // CW
        };

        static constexpr uint8_t VAL_CONTROL_CHANGE_3FH41H[3] = {
            0,     // STOPPED
            63,    // CCW
            65     // CW
        };

        static constexpr std::array<MIDI::messageType_t, static_cast<uint8_t>(type_t::AMOUNT)> INTERNAL_MSG_TO_MIDI_TYPE = {
            MIDI::messageType_t::CONTROL_CHANGE,          // CONTROL_CHANGE_7FH01H
            MIDI::messageType_t::CONTROL_CHANGE,          // CONTROL_CHANGE_3FH41H
            MIDI::messageType_t::PROGRAM_CHANGE,          // PROGRAM_CHANGE
            MIDI::messageType_t::CONTROL_CHANGE,          // CONTROL_CHANGE
            MIDI::messageType_t::INVALID,                 // PRESET_CHANGE
            MIDI::messageType_t::PITCH_BEND,              // PITCH_BEND
            MIDI::messageType_t::NRPN_7BIT,               // NRPN_7BIT
            MIDI::messageType_t::NRPN_14BIT,              // NRPN_14BIT
            MIDI::messageType_t::CONTROL_CHANGE_14BIT,    // CONTROL_CHANGE_14BIT
            MIDI::messageType_t::INVALID,                 // RESERVED
        };
    };
}    // namespace io

#else
#include "stub/Encoders.h"
#endif