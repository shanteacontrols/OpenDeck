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

#include "Config.h"

typedef enum
{
    BIT_PARAMETER,
    BYTE_PARAMETER,
    WORD_PARAMETER
} sectionParameterType_t;

///
/// \brief A structure holding information for a single section.
///
typedef struct
{
    uint16_t parameters;
    sectionParameterType_t parameterType;
    bool preserveOnPartialReset;
    uint16_t defaultValue;
} dbSection_t;

///
/// \brief A structure holding information for a single block.
///
typedef struct
{
    uint8_t sections;
    uint16_t blockStartAddress;
    uint16_t sectionAddress[MAX_SECTIONS];
    dbSection_t section[MAX_SECTIONS];
} dbBlock_t;

typedef enum
{
    initPartial,
    initWipe
} initType_t;