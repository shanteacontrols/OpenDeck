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
    buttonNone,
    buttonChangePreset,
    buttonCustomHook,
    BUTTON_MESSAGE_TYPES
} buttonMIDImessage_t;

/// @}
