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

#include "database/blocks/Analog.h"

typedef enum
{
    sysExSection_analog_enable,
    sysExSection_analog_invert,
    sysExSection_analog_type,
    sysExSection_analog_midiID_LSB,
    sysExSection_analog_midiID_MSB,
    sysExSection_analog_lowerLimit_LSB,
    sysExSection_analog_lowerLimit_MSB,
    sysExSection_analog_upperLimit_LSB,
    sysExSection_analog_upperLimit_MSB,
    sysExSection_analog_midiChannel,
    SYSEX_SECTIONS_ANALOG
} sysExSection_analog_t;

//map sysex sections to sections in db
const uint8_t sysEx2DB_analog[SYSEX_SECTIONS_ANALOG] =
{
    dbSection_analog_enable,
    dbSection_analog_invert,
    dbSection_analog_type,
    dbSection_analog_midiID,
    dbSection_analog_midiID,
    dbSection_analog_lowerLimit,
    dbSection_analog_lowerLimit,
    dbSection_analog_upperLimit,
    dbSection_analog_upperLimit,
    dbSection_analog_midiChannel
};