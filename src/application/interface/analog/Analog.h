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
    Analog(Board &board, Database &database, MIDI &midi, LEDs &leds) :
    #else
    Analog(Board &board, Database &database, MIDI &midi, LEDs &leds, Display &display) :
    #endif
    #else
    #ifdef DISPLAY_SUPPORTED
    Analog(Board &board, Database &database, MIDI &midi, Display &display) :
    #else
    Analog(Board &board, Database &database, MIDI &midi) :
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
    {
        buttonHandler = nullptr;
        cinfoHandler = nullptr;
    }

    void update();
    void debounceReset(uint16_t index);
    void setButtonHandler(void(*fptr)(uint8_t adcIndex, uint16_t adcValue));
    void setCinfoHandler(bool(*fptr)(dbBlockID_t dbBlock, sysExParameter_t componentID));

    private:
    void checkPotentiometerValue(analogType_t analogType, uint8_t analogID, uint16_t value);
    void checkFSRvalue(uint8_t analogID, uint16_t pressure);
    bool fsrPressureStable(uint8_t analogID);
    bool getFsrPressed(uint8_t fsrID);
    void setFsrPressed(uint8_t fsrID, bool state);
    bool getFsrDebounceTimerStarted(uint8_t fsrID);
    void setFsrDebounceTimerStarted(uint8_t fsrID, bool state);

    Board       &board;
    Database    &database;
    MIDI        &midi;
    #ifdef LEDS_SUPPORTED
    LEDs        &leds;
    #endif
    #ifdef DISPLAY_SUPPORTED
    Display     &display;
    #endif

    void        (*buttonHandler)(uint8_t adcIndex, uint16_t adcValue);
    bool        (*cinfoHandler)(dbBlockID_t dbBlock, sysExParameter_t componentID);
    uint16_t    lastAnalogueValue[MAX_NUMBER_OF_ANALOG];
    uint8_t     fsrPressed[MAX_NUMBER_OF_ANALOG/8+1];
};

/// @}