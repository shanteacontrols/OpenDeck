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
            virtual uint8_t state(size_t index) = 0;
        };

        Encoders(HWA& hwa, Database& database, MIDI& midi, Display& display, ComponentInfo& cInfo)
            : _hwa(hwa)
            , _database(database)
            , _midi(midi)
            , _display(display)
            , _cInfo(cInfo)
        {}

        void       init();
        void       update();
        void       resetValue(uint8_t encoderID);
        void       setValue(uint8_t encoderID, uint16_t value);
        position_t read(uint8_t encoderID, uint8_t pairState);

        private:
        HWA&           _hwa;
        Database&      _database;
        MIDI&          _midi;
        Display&       _display;
        ComponentInfo& _cInfo;

        /// Time in milliseconds after which debounce mode is reset if encoder isn't moving.
        static constexpr uint32_t ENCODERS_DEBOUNCE_RESET_TIME = 50;

        /// Number of times movement in the same direction must be registered in order
        /// for debouncer to become active. Once the debouncer is active, all further changes
        /// in the movement will be ignored, that is, all movements will be registered in the
        /// direction which was repeated. This state is reset until the encoder is either stopped
        /// or if same amount of movements are registered in the opposite direction.
        static constexpr uint8_t ENCODERS_DEBOUNCE_COUNT = 4;

        /// Time threshold in milliseconds between two encoder steps used to detect fast movement.
        static constexpr uint32_t ENCODERS_SPEED_TIMEOUT = 140;

        /// Holds current MIDI value for all encoders.
        int16_t _midiValue[MAX_NUMBER_OF_ENCODERS] = { 0 };

        /// Array holding last movement time for all encoders.
        uint32_t _lastMovementTime[MAX_NUMBER_OF_ENCODERS] = {};

        /// Array holding current speed (in steps) for all encoders.
        uint8_t _encoderSpeed[MAX_NUMBER_OF_ENCODERS] = {};

        /// Array holding previous encoder direction for all encoders.
        position_t _lastDirection[MAX_NUMBER_OF_ENCODERS] = {};

        /// Array holding current debounced direction for all encoders.
        position_t _debounceDirection[MAX_NUMBER_OF_ENCODERS] = {};

        /// Used to detect constant rotation in single direction.
        /// Once n consecutive movements in same direction are detected,
        /// all further movements are assumed to have same direction until
        /// encoder stops moving for ENCODERS_DEBOUNCE_RESET_TIME milliseconds *or*
        /// n new consecutive movements are made in the opposite direction.
        /// n = ENCODERS_DEBOUNCE_COUNT (defined in Constants.h)
        uint8_t _debounceCounter[MAX_NUMBER_OF_ENCODERS] = {};

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