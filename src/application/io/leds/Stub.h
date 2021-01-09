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

namespace IO
{
    class LEDs
    {
        public:
        enum class rgbIndex_t : uint8_t
        {
            r,
            g,
            b
        };

        enum class color_t : uint8_t
        {
            off,
            red,
            green,
            yellow,
            blue,
            magenta,
            cyan,
            white,
            AMOUNT
        };

        enum class setting_t : uint8_t
        {
            blinkWithMIDIclock,
            fadeSpeed,
            useStartupAnimation,
            AMOUNT
        };

        enum class controlType_t : uint8_t
        {
            midiInNoteForStateCCforBlink,
            localNoteForStateNoBlink,
            midiInCCforStateNoteForBlink,
            localCCforStateNoBlink,
            midiInPCforStateNoBlink,
            localPCforStateNoBlink,
            midiInNoteForStateAndBlink,
            localNoteForStateAndBlink,
            midiInCCforStateAndBlink,
            localCCforStateAndBlink,
            AMOUNT
        };

        enum class blinkSpeed_t : uint8_t
        {
            noBlink,
            s100ms,
            s200ms,
            s300ms,
            s400ms,
            s500ms,
            s600ms,
            s700ms,
            s800ms,
            s900ms,
            s1000ms,
            AMOUNT
        };

        enum class blinkType_t : uint8_t
        {
            timer,
            midiClock
        };

        enum class brightness_t : uint8_t
        {
            bOff,
            b25,
            b50,
            b75,
            b100
        };

        class HWA
        {
            public:
            HWA() {}
            virtual void   setState(size_t index, brightness_t brightness)                         = 0;
            virtual size_t rgbSingleComponentIndex(size_t rgbIndex, LEDs::rgbIndex_t rgbComponent) = 0;
            virtual size_t rgbIndex(size_t singleLEDindex)                                         = 0;
            virtual void   setFadeSpeed(size_t transitionSpeed)                                    = 0;
        };

        LEDs(HWA& hwa, Database& database)
        {}

        void init(bool startUp = true)
        {
        }

        void checkBlinking(bool forceChange = false)
        {
        }

        void setAllOn()
        {
        }

        void setAllOff()
        {
        }

        void refresh()
        {
        }

        void setColor(uint8_t ledID, color_t color)
        {
        }

        color_t color(uint8_t ledID)
        {
            return color_t::off;
        }

        void setBlinkState(uint8_t ledID, blinkSpeed_t value)
        {
        }

        bool getBlinkState(uint8_t ledID)
        {
            return false;
        }

        size_t rgbSingleComponentIndex(size_t rgbIndex, LEDs::rgbIndex_t rgbComponent)
        {
            return 0;
        }

        size_t rgbIndex(size_t singleLEDindex)
        {
            return 0;
        }

        bool setFadeSpeed(uint8_t transitionSpeed)
        {
            return false;
        }

        void midiToState(MIDI::messageType_t messageType, uint8_t data1, uint8_t data2, uint8_t channel, bool local)
        {
        }

        void setBlinkType(blinkType_t blinkType)
        {
        }

        void resetBlinking()
        {
        }
    };
}    // namespace IO