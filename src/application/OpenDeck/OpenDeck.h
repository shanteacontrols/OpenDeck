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

#include "database/Database.h"
#include "interface/analog/Analog.h"
#ifdef LEDS_SUPPORTED
#include "interface/digital/output/leds/LEDs.h"
#endif
#ifdef DISPLAY_SUPPORTED
#include "interface/display/Display.h"
#endif
#ifdef TOUCHSCREEN_SUPPORTED
#include "interface/display/touch/Touchscreen.h"
#endif
#include "midi/src/MIDI.h"
#include "sysconfig/SysConfig.h"

class OpenDeck
{
    public:
    OpenDeck() :

    database(Board::memoryRead, Board::memoryWrite)
    #ifdef LEDS_SUPPORTED
    ,leds(database)
    #endif
    #ifdef LEDS_SUPPORTED
    #ifdef DISPLAY_SUPPORTED
    ,analog(database, midi, leds, display)
    #else
    ,analog(database, midi, leds)
    #endif
    #else
    #ifdef DISPLAY_SUPPORTED
    ,analog(database, midi, display)
    #else
    ,analog(database, midi)
    #endif
    #endif
    #ifdef LEDS_SUPPORTED
    #ifdef DISPLAY_SUPPORTED
    ,buttons(database, midi, leds, display)
    #else
    ,buttons(database, midi, leds)
    #endif
    #else
    #ifdef DISPLAY_SUPPORTED
    ,buttons(database, midi, display)
    #else
    ,buttons(database, midi)
    #endif
    #endif
    #ifdef LEDS_SUPPORTED
    #ifdef DISPLAY_SUPPORTED
    ,encoders(database, midi, leds, display)
    #else
    ,encoders(database, midi, leds)
    #endif
    #else
    #ifdef DISPLAY_SUPPORTED
    ,encoders(database, midi, display)
    #else
    ,encoders(database, midi)
    #endif
    #endif
    #ifdef LEDS_SUPPORTED
    #ifdef DISPLAY_SUPPORTED
    ,sysConfig(database, midi, buttons, encoders, analog, leds, display)
    #else
    ,sysConfig(database, midi, buttons, encoders, analog, leds)
    #endif
    #else
    #ifdef DISPLAY_SUPPORTED
    ,sysConfig(database, midi, buttons, encoders, analog, display)
    #else
    ,sysConfig(database, midi, buttons, encoders, analog)
    #endif
    #endif
    {}

    void init();
    void update();
    void checkMIDI();
    void checkComponents();

    private:
    MIDI                midi;
    Database            database;
    #ifdef LEDS_SUPPORTED
    LEDs                leds;
    #endif
    Analog              analog;
    Buttons             buttons;
    Encoders            encoders;
    #ifdef DISPLAY_SUPPORTED
    Display             display;
    #endif
    #ifdef TOUCHSCREEN_SUPPORTED
    Touchscreen         touchscreen;
    #endif
    SysConfig           sysConfig;
};