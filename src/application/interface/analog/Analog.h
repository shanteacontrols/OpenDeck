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

#include "board/Board.h"
#include "database/Database.h"
#include "midi/src/MIDI.h"
#ifdef LEDS_SUPPORTED
#include "interface/digital/output/leds/LEDs.h"
#endif
#ifdef DISPLAY_SUPPORTED
#include "interface/display/Display.h"
#endif
#include "DataTypes.h"
#include "sysex/src/DataTypes.h"

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
    Analog(Database &database, MIDI &midi, LEDs &leds) :
    #else
    Analog(Database &database, MIDI &midi, LEDs &leds, Display &display) :
    #endif
    #else
    #ifdef DISPLAY_SUPPORTED
    Analog(Database &database, MIDI &midi, Display &display) :
    #else
    Analog(Database &database, MIDI &midi) :
    #endif
    #endif
    database(database),
    midi(midi)
    #ifdef LEDS_SUPPORTED
    ,leds(leds)
    #endif
    #ifdef DISPLAY_SUPPORTED
    ,display(display)
    #endif
    {}

    void update();
    void debounceReset(uint16_t index);
    void setButtonHandler(void(*fptr)(uint8_t adcIndex, uint16_t adcValue));

    private:
    void checkPotentiometerValue(analogType_t analogType, uint8_t analogID, uint32_t value);
    void checkFSRvalue(uint8_t analogID, uint16_t pressure);
    bool fsrPressureStable(uint8_t analogID);
    bool getFsrPressed(uint8_t fsrID);
    void setFsrPressed(uint8_t fsrID, bool state);
    bool getFsrDebounceTimerStarted(uint8_t fsrID);
    void setFsrDebounceTimerStarted(uint8_t fsrID, bool state);
    uint32_t calibratePressure(uint32_t value, pressureType_t type);

    Database    &database;
    MIDI        &midi;
    #ifdef LEDS_SUPPORTED
    LEDs        &leds;
    #endif
    #ifdef DISPLAY_SUPPORTED
    Display     &display;
    #endif

    enum class potDirection_t : uint8_t
    {
        initial,
        left,
        right
    };

    void            (*buttonHandler)(uint8_t adcIndex, uint16_t adcValue) = nullptr;
    uint16_t        lastAnalogueValue[MAX_NUMBER_OF_ANALOG] = {};
    uint8_t         fsrPressed[MAX_NUMBER_OF_ANALOG] = {};
    potDirection_t  lastDirection[MAX_NUMBER_OF_ANALOG] = {};
};

/// @}