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
#include "midi/src/MIDI.h"
#include "io/display/Display.h"
#include "Constants.h"
#include "io/common/Common.h"
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
        {}

        void init()
        {
        }

        void update()
        {
        }

        void resetValue(uint8_t encoderID)
        {
        }

        void setValue(uint8_t encoderID, uint16_t value)
        {
        }

        position_t read(uint8_t encoderID, uint8_t pairState)
        {
            return position_t::stopped;
        }
    };
}    // namespace IO