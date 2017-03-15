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

#define MAX_NUMBER_OF_BLOCKS    7
#define MAX_NUMBER_OF_SECTIONS  7
#define MAX_CUSTOM_REQUESTS     10

#define INVALID_VALUE           128

#define PARAMETERS_PER_MESSAGE  32

//1 - one byte size for parameter index and new value (uint8_t)
//2 - two byte size (uint16_t)

#define PARAM_SIZE              1

//manufacturer ID bytes
#define SYS_EX_M_ID_0           0x00
#define SYS_EX_M_ID_1           0x53
#define SYS_EX_M_ID_2           0x43

#if PARAM_SIZE == 2
typedef int16_t sysExParameter_t;
#elif PARAM_SIZE == 1
typedef int8_t sysExParameter_t;
#else
#error Incorrect parameter size for SysEx
#endif