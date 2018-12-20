/*

Copyright 2015-2018 Igor Petrovic

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
/// \brief Button handling.
/// \defgroup interfaceButtons Buttons
/// \ingroup interfaceDigitalIn
/// @{

class Buttons
{
    public:
    #ifdef LEDS_SUPPORTED
    #ifdef DISPLAY_SUPPORTED
    Buttons(Database &database, MIDI &midi, LEDs &leds, Display &display) :
    #else
    Buttons(Database &database, MIDI &midi, LEDs &leds) :
    #endif
    #else
    #ifdef DISPLAY_SUPPORTED
    Buttons(Database &database, MIDI &midi, Display &display) :
    #else
    Buttons(Database &database, MIDI &midi) :
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
    bool getStateFromAnalogValue(uint16_t adcValue);
    void processButton(uint8_t buttonID, bool state);
    bool getButtonState(uint8_t buttonID);

    private:
    void sendMessage(uint8_t buttonID, bool state, buttonMIDImessage_t buttonMessage = BUTTON_MESSAGE_TYPES);
    void setButtonState(uint8_t buttonID, uint8_t state);
    void setLatchingState(uint8_t buttonID, uint8_t state);
    bool getLatchingState(uint8_t buttonID);
    bool buttonDebounced(uint8_t buttonID, bool state);

    Database    &database;
    MIDI        &midi;
    #ifdef LEDS_SUPPORTED
    LEDs        &leds;
    #endif
    #ifdef DISPLAY_SUPPORTED
    Display     &display;
    #endif

    ///
    /// \brief Array holding debounce count for all buttons to avoid incorrect state detection.
    ///
    uint8_t     buttonDebounceCounter[MAX_NUMBER_OF_BUTTONS+MAX_NUMBER_OF_ANALOG+MAX_TOUCHSCREEN_BUTTONS] = {};

    ///
    /// \brief Array holding current state for all buttons.
    ///
    uint8_t     buttonPressed[MAX_NUMBER_OF_BUTTONS+MAX_NUMBER_OF_ANALOG+MAX_TOUCHSCREEN_BUTTONS] = {};

    ///
    /// \brief Array holding last sent state for latching buttons only.
    ///
    uint8_t     lastLatchingState[MAX_NUMBER_OF_BUTTONS+MAX_NUMBER_OF_ANALOG+MAX_TOUCHSCREEN_BUTTONS] = {};

    ///
    /// \brief Array used for simpler building of transport control messages.
    /// Based on MIDI specification for transport control.
    ///
    uint8_t     mmcArray[6] =  { 0xF0, 0x7F, 0x7F, 0x06, 0x00, 0xF7 };
};

/// @}