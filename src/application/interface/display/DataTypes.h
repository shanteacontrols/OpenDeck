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