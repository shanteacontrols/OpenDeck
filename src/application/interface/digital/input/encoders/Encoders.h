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
#include "Constants.h"

///
/// \brief Encoder handling.
/// \defgroup interfaceEncoders Encoders
/// \ingroup interfaceDigitalIn
/// @{

class Encoders
{
    public:
    #ifdef LEDS_SUPPORTED
    #ifdef DISPLAY_SUPPORTED
    Encoders(Database &database, MIDI &midi, LEDs &leds, Display &display) :
    #else
    Encoders(Database &database, MIDI &midi, LEDs &leds) :
    #endif
    #else
    #ifdef DISPLAY_SUPPORTED
    Encoders(Database &database, MIDI &midi, Display &display) :
    #else
    Encoders(Database &database, MIDI &midi) :
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

    void init();
    void update();
    void resetValue(uint8_t encoderID);
    void setValue(uint8_t encoderID, uint16_t value);

    private:
    Database            &database;
    MIDI                &midi;
    #ifdef LEDS_SUPPORTED
    LEDs        &leds;
    #endif
    #ifdef DISPLAY_SUPPORTED
    Display             &display;
    #endif

    ///
    /// \brief Holds current MIDI value for all encoders.
    ///
    int16_t             midiValue[MAX_NUMBER_OF_ENCODERS] = { 0 };

    ///
    /// \brief Array holding last movement time for all encoders.
    ///
    uint32_t            lastMovementTime[MAX_NUMBER_OF_ENCODERS] = {};

    ///
    /// \brief Array holding current speed (in steps) for all encoders.
    ///
    uint8_t             encoderSpeed[MAX_NUMBER_OF_ENCODERS] = {};

    ///
    /// \brief Array holding previous encoder direction for all encoders.
    ///
    encoderPosition_t   lastDirection[MAX_NUMBER_OF_ENCODERS] = {};

    ///
    /// \brief Array holding current debounced direction for all encoders.
    ///
    encoderPosition_t   debounceDirection[MAX_NUMBER_OF_ENCODERS] = {};

    ///
    /// \brief Used to detect constant rotation in single direction.
    /// Once n consecutive movements in same direction are detected,
    /// all further movements are assumed to have same direction until
    /// encoder stops moving for DEBOUNCE_RESET_TIME milliseconds *or*
    /// n new consecutive movements are made in the opposite direction.
    /// n = ENCODER_DEBOUNCE_COUNT (defined in Constants.h)
    ///
    uint8_t             debounceCounter[MAX_NUMBER_OF_ENCODERS] = {};
};

/// @}