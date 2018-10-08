/*
    OpenDeck MIDI platform firmware
    Copyright (C) 2015-2018 Igor Petrovic

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
    Buttons(Board &board, Database &database, MIDI &midi, LEDs &leds, Display &display) :
    #else
    Buttons(Board &board, Database &database, MIDI &midi, LEDs &leds) :
    #endif
    #else
    #ifdef DISPLAY_SUPPORTED
    Buttons(Board &board, Database &database, MIDI &midi, Display &display) :
    #else
    Buttons(Board &board, Database &database, MIDI &midi) :
    #endif
    #endif
    board(board),
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
    void setCinfoHandler(bool(*fptr)(dbBlockID_t dbBlock, sysExParameter_t componentID));

    private:
    void sendMessage(uint8_t buttonID, bool state, buttonMIDImessage_t buttonMessage = BUTTON_MESSAGE_TYPES);
    void setButtonState(uint8_t buttonID, uint8_t state);
    void setLatchingState(uint8_t buttonID, uint8_t state);
    bool getLatchingState(uint8_t buttonID);
    bool buttonDebounced(uint8_t buttonID, bool state);

    Board       &board;
    Database    &database;
    MIDI        &midi;
    #ifdef LEDS_SUPPORTED
    LEDs        &leds;
    #endif
    #ifdef DISPLAY_SUPPORTED
    Display     &display;
    #endif

    bool        (*cinfoHandler)(dbBlockID_t dbBlock, sysExParameter_t componentID) = nullptr;

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