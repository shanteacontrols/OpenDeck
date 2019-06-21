/*

Copyright 2015-2019 Igor Petrovic

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
#include "Constants.h"
#include "midi/src/MIDI.h"

namespace Interface
{
    namespace digital
    {
        namespace output
        {
            ///
            /// \brief LED handling.
            /// \defgroup leds LEDs
            /// \ingroup interface
            /// @{
            ///
            class LEDs
            {
                public:
                LEDs(Database& database)
                    : database(database)
                {}

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

                void           init(bool startUp = true);
                void           checkBlinking(bool forceChange = false);
                void           setAllOn();
                void           setAllOff();
                void           setColor(uint8_t ledID, color_t color);
                color_t        getColor(uint8_t ledID);
                void           setBlinkState(uint8_t ledID, blinkSpeed_t value);
                bool           getBlinkState(uint8_t ledID);
                bool           setFadeTime(uint8_t transitionSpeed);
                void           midiToState(MIDI::messageType_t messageType, uint8_t data1, uint8_t data2, uint8_t channel, bool local = false);
                void           setBlinkType(blinkType_t blinkType);
                blinkType_t    getBlinkType();
                void           resetBlinking();
                static uint8_t getLEDstate(uint8_t ledID);

                private:
                color_t      valueToColor(uint8_t receivedVelocity);
                blinkSpeed_t valueToBlinkSpeed(uint8_t value);
                uint8_t      getState(uint8_t ledID);
                void         handleLED(uint8_t ledID, bool state, bool rgbLED = false, rgbIndex_t index = rgbIndex_t::r);
                void         startUpAnimation();

                Database& database;

                ///
                /// \brief Array holding time after which LEDs should blink.
                ///
                uint8_t blinkTimer[MAX_NUMBER_OF_LEDS] = {};

                ///
                /// \brief Holds currently active LED blink type.
                ///
                blinkType_t ledBlinkType = blinkType_t::timer;

                ///
                /// \brief Pointer to array used to check if blinking LEDs should toggle state.
                ///
                const uint8_t* blinkResetArrayPtr = nullptr;

                ///
                /// \brief Array holding MIDI clock pulses after which LED state is toggled for all possible blink rates.
                ///
                const uint8_t blinkReset_midiClock[static_cast<uint8_t>(blinkSpeed_t::AMOUNT)] = {
                    255,    //no blinking
                    2,
                    3,
                    4,
                    6,
                    9,
                    12,
                    18,
                    24,
                    36,
                    48
                };

                ///
                /// \brief Array holding time indexes (multipled by 100) after which LED state is toggled for all possible blink rates.
                ///
                const uint8_t blinkReset_timer[static_cast<uint8_t>(blinkSpeed_t::AMOUNT)] = {
                    0,
                    1,
                    2,
                    3,
                    4,
                    5,
                    6,
                    7,
                    8,
                    9,
                    10
                };

                ///
                /// \brief Array used to determine when the blink state for specific blink rate should be changed.
                ///
                uint8_t blinkCounter[static_cast<uint8_t>(blinkSpeed_t::AMOUNT)] = {};

                ///
                /// \brief Holds last time in miliseconds when LED blinking has been updated.
                ///
                uint32_t lastLEDblinkUpdateTime = 0;

                ///
                // \brief Holds blink state for each blink speed so that leds are in sync.
                ///
                bool blinkState[static_cast<uint8_t>(blinkSpeed_t::AMOUNT)] = {};
            };
        }    // namespace output
    }        // namespace digital
}    // namespace Interface