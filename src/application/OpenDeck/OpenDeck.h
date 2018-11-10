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