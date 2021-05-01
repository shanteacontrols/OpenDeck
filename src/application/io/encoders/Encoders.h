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

#ifndef ENCODERS_SUPPORTED
#include "Stub.h"
#else

#include "database/Database.h"
#include "midi/src/MIDI.h"
#include "io/display/Display.h"
#include "io/common/CInfo.h"

namespace IO
{
    class Encoders
    {
        public:
        enum class type_t : uint8_t
        {
            t7Fh01h,
            t3Fh41h,
            tProgramChange,
            tControlChange,
            tPresetChange,
            tPitchBend,
            tNRPN7bit,
            tNRPN14bit,
            tControlChange14bit,
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
            //should return true if the value has been refreshed, false otherwise
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

        Encoders(HWA&           hwa,
                 Filter&        filter,
                 uint32_t       timeDiffTimeout,
                 Database&      database,
                 MIDI&          midi,
                 Display&       display,
                 ComponentInfo& cInfo)
            : _hwa(hwa)
            , _filter(filter)
            , TIME_DIFF_READOUT(timeDiffTimeout)
            , _database(database)
            , _midi(midi)
            , _display(display)
            , _cInfo(cInfo)
        {}

        void       init();
        void       update();
        void       resetValue(size_t index);
        void       setValue(size_t index, uint16_t value);
        position_t read(size_t index, uint8_t pairState);

        private:
        HWA&    _hwa;
        Filter& _filter;

        /// Time difference betweeen multiple encoder readouts in milliseconds.
        const uint32_t TIME_DIFF_READOUT;

        Database&      _database;
        MIDI&          _midi;
        Display&       _display;
        ComponentInfo& _cInfo;

        void processReading(size_t index, uint8_t pairValue, uint32_t sampleTime);

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
            0,     //0000
            1,     //0001
            -1,    //0010
            0,     //0011
            -1,    //0100
            0,     //0101
            0,     //0110
            1,     //0111
            1,     //1000
            0,     //1001
            0,     //1010
            -1,    //1011
            0,     //1100
            -1,    //1101
            1,     //1110
            0      //1111
        };

        /// Used to achieve linear encoder acceleration on fast movement.
        /// Every time fast movement is detected, amount of steps is increased by this value.
        /// Used only in CC/Pitch bend/NRPN modes. In Pitch bend/NRPN modes, this value is multiplied
        /// by 4 due to a larger value range.
        const uint8_t _encoderSpeedChange[static_cast<uint8_t>(IO::Encoders::acceleration_t::AMOUNT)] = {
            0,    //acceleration disabled
            1,
            2,
            3
        };

        /// Maximum value by which MIDI value is increased during acceleration.
        const uint8_t _encoderMaxAccSpeed[static_cast<uint8_t>(IO::Encoders::acceleration_t::AMOUNT)] = {
            0,    //acceleration disabled
            5,
            10,
            100
        };

        /// Array used for easier access to current encoder MIDI value in 7Fh01h and 3Fh41h modes.
        /// Matched with type_t and position_t
        const uint8_t _encValue[2][3] = {
            //t7Fh01h
            {
                0,      //stopped
                127,    //ccw
                1       //cw
            },

            //t3Fh41h
            {
                0,     //stopped
                63,    //ccw
                65     //cw
            }
        };
    };
}    // namespace IO

#endif