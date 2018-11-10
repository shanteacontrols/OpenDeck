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
/// \brief List of all possible text types on display.
///
typedef enum
{
    lcdtext_still,
    lcdText_temp
} lcdTextType_t;

///
/// \brief List of all possible text scrolling directions.
///
typedef enum
{
    scroll_ltr,
    scroll_rtl
} scrollDirection_t;

///
/// \brief Structure holding data for scrolling event on display for single row.
///
typedef struct
{
    uint8_t size;
    uint8_t startIndex;
    int8_t currentIndex;
    scrollDirection_t direction;
} scrollEvent_t;

///
/// \brief Same enumeration as midiMessageType_t but without custom values.
/// Used for easier access to event strings stored in arrays
///
typedef enum
{
    midiMessageNoteOff_display,
    midiMessageNoteOn_display,
    midiMessageControlChange_display,
    midiMessageProgramChange_display,
    midiMessageAfterTouchChannel_display,
    midiMessageAfterTouchPoly_display,
    midiMessagePitchBend_display,
    midiMessageSystemExclusive_display,
    midiMessageTimeCodeQuarterFrame_display,
    midiMessageSongPosition_display,
    midiMessageSongSelect_display,
    midiMessageTuneRequest_display,
    midiMessageClock_display,
    midiMessageStart_display,
    midiMessageContinue_display,
    midiMessageStop_display,
    midiMessageActiveSensing_display,
    midiMessageSystemReset_display,
    //these messages aren't part of regular midiMessageType_t enum
    midiMessageMMCplay_display,
    midiMessageMMCstop_display,
    midiMessageMMCpause_display,
    midiMessageMMCrecordOn_display,
    midiMessageMMCrecordOff_display,
    midiMessageNRPN_display,
    messagePresetChange_display
} messageTypeDisplay_t;