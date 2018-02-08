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

#include "../../../../database/Database.h"

//define extra sysex sections for setting/getting led states
#define ledColorSection                 LED_SECTIONS
#define ledBlinkSection                 LED_SECTIONS+1

#define BLINK_TIME_MIN                  0x02
#define BLINK_TIME_MAX                  0x0F

#define FADE_TIME_MIN                   0x00
#define FADE_TIME_MAX                   0x0A

#define LED_ACTIVE_BIT                  0x00
#define LED_BLINK_ON_BIT                0x01
#define LED_STATE_BIT                   0x02
#define LED_RGB_BIT                     0x03
#define LED_RGB_R_BIT                   0x04
#define LED_RGB_G_BIT                   0x05
#define LED_RGB_B_BIT                   0x06
