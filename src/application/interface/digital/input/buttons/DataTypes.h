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

///
/// \ingroup interfaceButtons
/// @{


///
/// \brief List of all possible button types.
///
typedef enum
{
    buttonMomentary,    ///< Event on press and release.
    buttonLatching,     ///< Event between presses only.
    BUTTON_TYPES        ///< TOtal number of button types.
} buttonType_t;

///
/// \brief List of all possible MIDI messages buttons can send.
///
typedef enum
{
    buttonNote,
    buttonPC,
    buttonCC,
    buttonCCreset,
    buttonMMCStop,
    buttonMMCPlay,
    buttonMMCRecord,
    buttonMMCPause,
    buttonRealTimeClock,
    buttonRealTimeStart,
    buttonRealTimeContinue,
    buttonRealTimeStop,
    buttonRealTimeActiveSensing,
    buttonRealTimeSystemReset,
    buttonPCinc,
    buttonPCdec,
    BUTTON_MESSAGE_TYPES
} buttonMIDImessage_t;

/// @}
