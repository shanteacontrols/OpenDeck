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
#include "io/leds/LEDs.h"
#include "io/display/Display.h"
#include "io/common/Common.h"
#include "io/common/CInfo.h"

namespace IO
{
    class Buttons
    {
        public:
        enum class type_t : uint8_t
        {
            momentary,    ///< Event on press and release.
            latching,     ///< Event between presses only.
            AMOUNT        ///< Total number of button types.
        };

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
        {}

        void update(bool forceResend = false)
        {
        }

        void processButton(size_t index, bool state)
        {
        }

        bool state(size_t index)
        {
            return false;
        }

        void reset(size_t index)
        {
        }
    };
}    // namespace IO