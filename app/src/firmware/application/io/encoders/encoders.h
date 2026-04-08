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
#include "application/io/base.h"
#include "application/messaging/messaging.h"
#include "application/global/midi_program.h"
#include "application/system/config.h"
#include "application/io/base.h"
#include "application/protocol/midi/midi.h"

#include <optional>

namespace io::encoders
{
    class Encoders : public io::Base
    {
        public:
        Encoders(Hwa&      hwa,
                 Filter&   filter,
                 Database& database,
                 uint32_t  timeDiffTimeout = 1);

        bool   init() override;
        void   updateSingle(size_t index, bool forceRefresh = false) override;
        void   updateAll(bool forceRefresh = false) override;
        size_t maxComponentUpdateIndex() override;
        void   reset(size_t index);

        private:
        struct Descriptor
        {
            type_t           type  = type_t::CONTROL_CHANGE_7FH01H;
            messaging::Event event = {};
        };

        using ValueIncDecMIDI7Bit  = util::IncDec<uint8_t, 0, protocol::midi::MAX_VALUE_7BIT>;
        using ValueIncDecMIDI14Bit = util::IncDec<uint16_t, 0, protocol::midi::MAX_VALUE_14BIT>;

        /// Time threshold in milliseconds between two encoder steps used to detect fast movement.
        static constexpr uint32_t ENCODERS_SPEED_TIMEOUT = 140;

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
        static constexpr uint8_t ENCODER_SPEED_CHANGE[static_cast<uint8_t>(acceleration_t::AMOUNT)] = {
            0,    // acceleration disabled
            1,
            2,
            3
        };

        /// Value by which value is increased during acceleration.
        static constexpr uint8_t ENCODER_ACC_STEP_INC[static_cast<uint8_t>(acceleration_t::AMOUNT)] = {
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

        static constexpr uint8_t VAL_CONTROL_CHANGE_41H01H[3] = {
            0,     // STOPPED
            65,    // CCW
            1      // CW
        };

        static constexpr std::array<protocol::midi::messageType_t, static_cast<uint8_t>(type_t::AMOUNT)> INTERNAL_MSG_TO_MIDI_TYPE = {
            protocol::midi::messageType_t::CONTROL_CHANGE,          // CONTROL_CHANGE_7FH01H
            protocol::midi::messageType_t::CONTROL_CHANGE,          // CONTROL_CHANGE_3FH41H
            protocol::midi::messageType_t::PROGRAM_CHANGE,          // PROGRAM_CHANGE
            protocol::midi::messageType_t::CONTROL_CHANGE,          // CONTROL_CHANGE
            protocol::midi::messageType_t::INVALID,                 // PRESET_CHANGE
            protocol::midi::messageType_t::PITCH_BEND,              // PITCH_BEND
            protocol::midi::messageType_t::NRPN_7BIT,               // NRPN_7BIT
            protocol::midi::messageType_t::NRPN_14BIT,              // NRPN_14BIT
            protocol::midi::messageType_t::CONTROL_CHANGE_14BIT,    // CONTROL_CHANGE_14BIT
            protocol::midi::messageType_t::CONTROL_CHANGE,          // RESERVED
            protocol::midi::messageType_t::INVALID,                 // BPM_CHANGE
            protocol::midi::messageType_t::NOTE_ON,                 // SINGLE_NOTE_VARIABLE_VAL
            protocol::midi::messageType_t::NOTE_ON,                 // SINGLE_NOTE_FIXED_VAL_BOTH_DIR
            protocol::midi::messageType_t::NOTE_ON,                 // SINGLE_NOTE_FIXED_VAL_ONE_DIR_0_OTHER_DIR
            protocol::midi::messageType_t::NOTE_ON,                 // TWO_NOTE_FIXED_VAL_BOTH_DIR
        };

        Hwa&      _hwa;
        Filter&   _filter;
        Database& _database;

        /// Time difference betweeen multiple encoder readouts in milliseconds.
        const uint32_t TIME_DIFF_READOUT;

        /// Holds current value for all encoders.
        int16_t _value[Collection::SIZE()] = { 0 };

        /// Array holding current speed (in steps) for all encoders.
        uint8_t _encoderSpeed[Collection::SIZE()] = {};

        /// Array holding last two readings from encoder pins.
        uint8_t _encoderData[Collection::SIZE()] = {};

        /// Array holding current amount of pulses for all encoders.
        int8_t _encoderPulses[Collection::SIZE()] = {};

        void                   fillDescriptor(size_t index, position_t position, Descriptor& descriptor);
        position_t             read(size_t index, uint8_t pairState);
        void                   processReading(size_t index, uint8_t pairValue, uint32_t sampleTime);
        void                   sendMessage(size_t index, position_t position, Descriptor& descriptor);
        void                   setValue(size_t index, uint16_t value);
        std::optional<uint8_t> sysConfigGet(sys::Config::Section::encoder_t section, size_t index, uint16_t& value);
        std::optional<uint8_t> sysConfigSet(sys::Config::Section::encoder_t section, size_t index, uint16_t value);
    };
}    // namespace io::encoders