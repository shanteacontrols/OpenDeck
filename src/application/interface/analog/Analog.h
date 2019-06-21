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
#include "midi/src/MIDI.h"
#ifdef LEDS_SUPPORTED
#include "interface/digital/output/leds/LEDs.h"
#endif
#ifdef DISPLAY_SUPPORTED
#include "interface/display/Display.h"
#endif
#include "interface/CInfo.h"

namespace Interface
{
    namespace analog
    {
        ///
        /// \brief Analog components handling.
        /// \defgroup analog Analog
        /// \ingroup interface
        /// @{
        ///

        class Analog
        {
            public:
#ifdef LEDS_SUPPORTED
#ifndef DISPLAY_SUPPORTED
            Analog(Database& database, MIDI& midi, Interface::digital::output::LEDs& leds, ComponentInfo& cInfo)
                :
#else
            Analog(Database& database, MIDI& midi, Interface::digital::output::LEDs& leds, Display& display, ComponentInfo& cInfo)
                :
#endif
#else
#ifdef DISPLAY_SUPPORTED
            Analog(Database& database, MIDI& midi, Display& display, ComponentInfo& cInfo)
                :
#else
            Analog(Database& database, MIDI& midi, ComponentInfo& cInfo)
                :
#endif
#endif
                database(database)
                , midi(midi)
#ifdef LEDS_SUPPORTED
                , leds(leds)
#endif
#ifdef DISPLAY_SUPPORTED
                , display(display)
#endif
                , cInfo(cInfo)
            {}

            enum class type_t : uint8_t
            {
                potentiometerControlChange,
                potentiometerNote,
                fsr,
                button,
                nrpn7b,
                nrpn14b,
                pitchBend,
                AMOUNT
            };

            enum class pressureType_t : uint8_t
            {
                velocity,
                aftertouch
            };

            void update();
            void debounceReset(uint16_t index);
            void setButtonHandler(void (*fptr)(uint8_t adcIndex, uint16_t adcValue));

            private:
            enum class potDirection_t : uint8_t
            {
                initial,
                decreasing,
                increasing
            };

            uint16_t getHysteresisValue(uint8_t analogID, int16_t value);
            void     checkPotentiometerValue(type_t analogType, uint8_t analogID, uint32_t value);
            void     checkFSRvalue(uint8_t analogID, uint16_t pressure);
            bool     fsrPressureStable(uint8_t analogID);
            bool     getFsrPressed(uint8_t fsrID);
            void     setFsrPressed(uint8_t fsrID, bool state);
            bool     getFsrDebounceTimerStarted(uint8_t fsrID);
            void     setFsrDebounceTimerStarted(uint8_t fsrID, bool state);
            uint32_t calibratePressure(uint32_t value, pressureType_t type);

            Database& database;
            MIDI&     midi;
#ifdef LEDS_SUPPORTED
            Interface::digital::output::LEDs& leds;
#endif
#ifdef DISPLAY_SUPPORTED
            Display& display;
#endif
            ComponentInfo& cInfo;

            void (*buttonHandler)(uint8_t adcIndex, uint16_t adcValue) = nullptr;
            uint16_t       lastAnalogueValue[MAX_NUMBER_OF_ANALOG] = {};
            uint8_t        fsrPressed[MAX_NUMBER_OF_ANALOG] = {};
            potDirection_t lastDirection[MAX_NUMBER_OF_ANALOG] = {};
        };

        /// @}
    }    // namespace analog
}    // namespace Interface