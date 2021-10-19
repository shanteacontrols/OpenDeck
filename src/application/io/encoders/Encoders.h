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

#ifndef ENCODERS_SUPPORTED
#include "stub/Encoders.h"
#else

namespace IO
{
    class Encoders
    {
        public:
        enum class type_t : uint8_t
        {
            controlChange7Fh01h,
            controlChange3Fh41h,
            programChange,
            controlChange,
            presetChange,
            pitchBend,
            nrpn7bit,
            nrpn14bit,
            controlChange14bit,
            AMOUNT
        };

        enum class position_t : uint8_t
        {
            stopped,
            ccw,
            cw,
        };

        enum class acceleration_t : uint8_t
        {
            disabled,
            slow,
            medium,
            fast,
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
            virtual bool isFiltered(size_t                    index,
                                    IO::Encoders::position_t  position,
                                    IO::Encoders::position_t& filteredPosition,
                                    uint32_t                  sampleTakenTime) = 0;

            virtual void     reset(size_t index)            = 0;
            virtual uint32_t lastMovementTime(size_t index) = 0;
        };

        Encoders(HWA&                     hwa,
                 Filter&                  filter,
                 uint32_t                 timeDiffTimeout,
                 Database&                database,
                 Util::MessageDispatcher& dispatcher);

        void update();
        void resetValue(size_t index);

        private:
        HWA&    _hwa;
        Filter& _filter;

        struct encoderDescriptor_t
        {
            type_t                             type          = type_t::controlChange7Fh01h;
            uint8_t                            pulsesPerStep = 0;
            Util::MessageDispatcher::message_t dispatchMessage;

            encoderDescriptor_t() = default;
        };

        /// Time difference betweeen multiple encoder readouts in milliseconds.
        const uint32_t TIME_DIFF_READOUT;

        Database&                _database;
        Util::MessageDispatcher& _dispatcher;

        void       fillEncoderDescriptor(size_t index, encoderDescriptor_t& descriptor);
        position_t read(size_t index, uint8_t pairState);
        void       processReading(size_t index, uint8_t pairValue, uint32_t sampleTime);
        void       sendMessage(size_t index, encoderDescriptor_t& descriptor);
        void       setValue(size_t index, uint16_t value);

        /// Time threshold in milliseconds between two encoder steps used to detect fast movement.
        static constexpr uint32_t ENCODERS_SPEED_TIMEOUT = 140;

        /// Holds current MIDI value for all encoders.
        int16_t _midiValue[MAX_NUMBER_OF_ENCODERS] = { 0 };

        /// Array holding current speed (in steps) for all encoders.
        uint8_t _encoderSpeed[MAX_NUMBER_OF_ENCODERS] = {};

        /// Array holding last two readings from encoder pins.
        uint8_t _encoderData[MAX_NUMBER_OF_ENCODERS] = {};

        /// Array holding current amount of pulses for all encoders.
        int8_t _encoderPulses[MAX_NUMBER_OF_ENCODERS] = {};

        /// Lookup table used to convert encoder reading to pulses.
        const int8_t _encoderLookUpTable[16] = {
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
        const uint8_t _encoderSpeedChange[static_cast<uint8_t>(IO::Encoders::acceleration_t::AMOUNT)] = {
            0,    // acceleration disabled
            1,
            2,
            3
        };

        /// Maximum value by which MIDI value is increased during acceleration.
        const uint8_t _encoderMaxAccSpeed[static_cast<uint8_t>(IO::Encoders::acceleration_t::AMOUNT)] = {
            0,    // acceleration disabled
            5,
            10,
            100
        };

        /// Array used for easier access to current encoder MIDI value in 7Fh01h and 3Fh41h modes.
        /// Matched with type_t and position_t
        const uint8_t _encValue[2][3] = {
            // controlChange7Fh01h
            {
                0,      // stopped
                127,    // ccw
                1       // cw
            },

            // controlChange3Fh41h
            {
                0,     // stopped
                63,    // ccw
                65     // cw
            }
        };

        const MIDI::messageType_t _internalMsgToMIDIType[static_cast<uint8_t>(type_t::AMOUNT)] = {
            MIDI::messageType_t::controlChange,
            MIDI::messageType_t::controlChange,
            MIDI::messageType_t::programChange,
            MIDI::messageType_t::controlChange,
            MIDI::messageType_t::invalid,
            MIDI::messageType_t::pitchBend,
            MIDI::messageType_t::nrpn7bit,
            MIDI::messageType_t::nrpn14bit,
            MIDI::messageType_t::controlChange14bit,
        };
    };
}    // namespace IO

#endif