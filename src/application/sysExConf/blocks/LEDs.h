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

#include "../../database/blocks/LEDs.h"

typedef enum
{
    sysExSection_leds_testColor,
    sysExSection_leds_testBlink,
    sysExSection_leds_hw,
    sysExSection_leds_activationID,
    sysExSection_leds_rgbEnable,
    sysExSection_leds_controlType,
    sysExSection_leds_activationValue,
    sysExSection_leds_midiChannel,
    SYSEX_SECTIONS_LEDS
} sysExSection_leds_t;

//map sysex sections to sections in db
const uint8_t sysEx2DB_leds[SYSEX_SECTIONS_LEDS] =
{
    0,
    0,
    dbSection_leds_hw,
    dbSection_leds_activationID,
    dbSection_leds_rgbEnable,
    dbSection_leds_controlType,
    dbSection_leds_activationValue,
    dbSection_leds_midiChannel,
};