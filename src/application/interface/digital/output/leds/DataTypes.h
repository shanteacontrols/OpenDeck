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