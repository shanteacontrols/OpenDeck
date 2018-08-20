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

typedef enum
{
    rgb_R,
    rgb_G,
    rgb_B
} rgbIndex_t;

typedef enum
{
    colorOff,
    colorRed,
    colorGreen,
    colorYellow,
    colorBlue,
    colorMagenta,
    colorCyan,
    colorWhite,
    LED_COLORS
} ledColor_t;

typedef enum
{
    ledGlobalParam_blinkMIDIclock,
    ledGlobalParam_fadeSpeed,
    ledGlobalParam_startUpRoutineState,
    LED_GLOBAL_PARAMETERS
} ledGlobalParam_t;

typedef enum
{
    ledControlMIDIin_noteStateCCblink,
    ledControlLocal_NoteStateOnly,
    ledControlMIDIin_CCstateNoteBlink,
    ledControlLocal_CCStateOnly,
    ledControlMIDIin_PCStateOnly,
    ledControlLocal_PCStateOnly,
    ledControlMIDIin_noteStateBlink,
    ledControlLocal_noteStateBlink,
    ledControlMIDIin_CCStateBlink,
    ledControlLocal_CCStateBlink,
    LED_CONTROL_TYPES
} ledControlType_t;

typedef enum
{
    blinkSpeed_noBlink,
    blinkSpeed_100ms,
    blinkSpeed_200ms,
    blinkSpeed_300ms,
    blinkSpeed_400ms,
    blinkSpeed_500ms,
    blinkSpeed_600ms,
    blinkSpeed_700ms,
    blinkSpeed_800ms,
    blinkSpeed_900ms,
    blinkSpeed_1000ms,
    BLINK_SPEEDS
} blinkSpeed_t;

typedef enum
{
    blinkType_timer,
    blinkType_midiClock
} blinkType_t;