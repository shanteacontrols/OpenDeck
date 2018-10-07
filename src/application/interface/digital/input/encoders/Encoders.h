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
#ifdef DISPLAY_SUPPORTED
#include "interface/display/Display.h"
#endif
#include "DataTypes.h"
#include "sysex/src/DataTypes.h"

///
/// \brief Encoder handling.
/// \defgroup interfaceEncoders Encoders
/// \ingroup interfaceDigitalIn
/// @{

class Encoders
{
    public:
    #ifdef DISPLAY_SUPPORTED
    Encoders(Board &board, Database &database, MIDI &midi, Display &display) :
    #else
    Encoders(Board &board, Database &database, MIDI &midi) :
    #endif
    board(board),
    database(database),
    midi(midi)
    #ifdef DISPLAY_SUPPORTED
    ,display(display)
    #endif
    {}

    void update();
    void setCinfoHandler(bool(*fptr)(dbBlockID_t dbBlock, sysExParameter_t componentID));

    private:
    bool (*cinfoHandler)(dbBlockID_t dbBlock, sysExParameter_t componentID);
    Board       &board;
    Database    &database;
    MIDI        &midi;
    #ifdef DISPLAY_SUPPORTED
    Display     &display;
    #endif
};

/// @}