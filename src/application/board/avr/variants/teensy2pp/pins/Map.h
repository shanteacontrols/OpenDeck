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

#include "../Hardware.h"
#include "Pins.h"

const uint8_t aInPinMap[MAX_NUMBER_OF_ANALOG] =
{
    AI_1_PIN,
    AI_2_PIN,
    AI_3_PIN,
    AI_4_PIN,
    AI_5_PIN,
    AI_6_PIN,
    AI_7_PIN,
    AI_8_PIN
};

volatile uint8_t *dInPortArray[] =
{
    &DI_1_PORT,
    &DI_2_PORT,
    &DI_3_PORT,
    &DI_4_PORT,
    &DI_5_PORT,
    &DI_6_PORT,
    &DI_7_PORT,
    &DI_8_PORT,
    &DI_9_PORT,
    &DI_10_PORT,
    &DI_11_PORT,
    &DI_12_PORT,
    &DI_13_PORT,
    &DI_14_PORT,
    &DI_15_PORT,
    &DI_16_PORT
};

const uint8_t dInPinArray[] =
{
    DI_1_PIN,
    DI_2_PIN,
    DI_3_PIN,
    DI_4_PIN,
    DI_5_PIN,
    DI_6_PIN,
    DI_7_PIN,
    DI_8_PIN,
    DI_9_PIN,
    DI_10_PIN,
    DI_11_PIN,
    DI_12_PIN,
    DI_13_PIN,
    DI_14_PIN,
    DI_15_PIN,
    DI_16_PIN
};

volatile uint8_t *dOutPortArray[] =
{
    &DO_1_PORT,
    &DO_2_PORT,
    &DO_3_PORT,
    &DO_4_PORT,
    &DO_5_PORT,
    &DO_6_PORT,
    &DO_7_PORT,
    &DO_8_PORT,
    &DO_9_PORT,
    &DO_10_PORT,
    &DO_11_PORT,
    &DO_12_PORT,
    &DO_13_PORT,
    &DO_14_PORT,
    &DO_15_PORT,
    &DO_16_PORT
};

const uint8_t dOutPinArray[] =
{
    DO_1_PIN,
    DO_2_PIN,
    DO_3_PIN,
    DO_4_PIN,
    DO_5_PIN,
    DO_6_PIN,
    DO_7_PIN,
    DO_8_PIN,
    DO_9_PIN,
    DO_10_PIN,
    DO_11_PIN,
    DO_12_PIN,
    DO_13_PIN,
    DO_14_PIN,
    DO_15_PIN,
    DO_16_PIN,
};