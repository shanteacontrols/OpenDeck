/*
    OpenDeck MIDI platform firmware
    Copyright (C) 2015-2017 Igor Petrovic

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
    ccLimitLow,
    ccLimitHigh
} ccLimitType_t;

typedef enum 
{
    aType_potentiometer_cc,
    aType_potentiometer_note,
    aType_fsr,
    aType_button,
    aType_NRPN_7,
    aType_NRPN_14,
    aType_NRPN_Inc,
    aType_NRPN_Dec,
    ANALOG_TYPES
} analogType_t;

typedef enum
{
    velocity,
    aftertouch
} pressureType_t;
