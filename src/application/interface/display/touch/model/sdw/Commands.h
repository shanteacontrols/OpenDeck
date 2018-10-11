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

///
/// \brief Various commands and IDs used to communicate with SDW display.
/// @{

#define START_BYTE                  0xAA

#define HANDSHAKE_BYTE1             0x00
#define HANDSHAKE_BYTE2             0x0A

#define END_CODE1                   0xCC
#define END_CODE2                   0x33
#define END_CODE3                   0xC3
#define END_CODE4                   0x3C

#define END_CODES                   4

#define PICTURE_DISPLAY             0x70

#define FULL_PICTURE_ID             0x00
#define SMALL_FORMAT_PICTURE_ID     0x01
#define CUT_PICTURE_ID              0x02

#define CUSTOM_ICON_DISPLAY         0x99

#define BACKLIGHT_ON_MAX            0x5F
#define BACKLIGHT_OFF               0x5E

#define PWM_BACKLIGHT_OFF           0x00
#define PWM_BACKLIGHT_MAX           0x64

#define BUTTON_INDEX_1              0x02
#define BUTTON_INDEX_2              0x03

#define COMMAND_ID_INDEX            0x01

#define BUTTON_ON_ID                0x79
#define BUTTON_OFF_ID               0x78

#define BUZZER_TIME_ID              0x79
#define BUZZER_TIME_MIN             0x01
#define BUZZER_TIME_MAX             0xFF

#define READ_DATA                   0x91

const uint8_t endCode[END_CODES] =
{
    END_CODE1,
    END_CODE2,
    END_CODE3,
    END_CODE4
};

/// @}