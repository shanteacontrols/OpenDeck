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

#include "Map.h"

mcuPin_t dInPins[MAX_NUMBER_OF_BUTTONS] =
{
    {
        .port = (uint8_t*)DI_1_PORT,
        .pin = DI_1_PIN,
    },

    {
        .port = (uint8_t*)DI_2_PORT,
        .pin = DI_2_PIN,
    },

    {
        .port = (uint8_t*)DI_3_PORT,
        .pin = DI_3_PIN,
    },

    {
        .port = (uint8_t*)DI_4_PORT,
        .pin = DI_4_PIN,
    },

    {
        .port = (uint8_t*)DI_5_PORT,
        .pin = DI_5_PIN,
    },

    {
        .port = (uint8_t*)DI_6_PORT,
        .pin = DI_6_PIN,
    },

    {
        .port = (uint8_t*)DI_7_PORT,
        .pin = DI_7_PIN,
    },

    {
        .port = (uint8_t*)DI_8_PORT,
        .pin = DI_8_PIN,
    },

    {
        .port = (uint8_t*)DI_9_PORT,
        .pin = DI_9_PIN,
    },

    {
        .port = (uint8_t*)DI_10_PORT,
        .pin = DI_10_PIN,
    },

    {
        .port = (uint8_t*)DI_11_PORT,
        .pin = DI_11_PIN,
    },

    {
        .port = (uint8_t*)DI_12_PORT,
        .pin = DI_12_PIN,
    },

    {
        .port = (uint8_t*)DI_13_PORT,
        .pin = DI_13_PIN,
    },

    {
        .port = (uint8_t*)DI_14_PORT,
        .pin = DI_14_PIN,
    },

    {
        .port = (uint8_t*)DI_15_PORT,
        .pin = DI_15_PIN,
    },

    {
        .port = (uint8_t*)DI_16_PORT,
        .pin = DI_16_PIN,
    }
};

mcuPin_t dOutPins[MAX_NUMBER_OF_LEDS] =
{
    {
        .port = (uint8_t*)DO_1_PORT,
        .pin = DO_1_PIN,
    },

    {
        .port = (uint8_t*)DO_2_PORT,
        .pin = DO_2_PIN,
    },

    {
        .port = (uint8_t*)DO_3_PORT,
        .pin = DO_3_PIN,
    },

    {
        .port = (uint8_t*)DO_4_PORT,
        .pin = DO_4_PIN,
    },

    {
        .port = (uint8_t*)DO_5_PORT,
        .pin = DO_5_PIN,
    },

    {
        .port = (uint8_t*)DO_6_PORT,
        .pin = DO_6_PIN,
    },

    {
        .port = (uint8_t*)DO_7_PORT,
        .pin = DO_7_PIN,
    },

    {
        .port = (uint8_t*)DO_8_PORT,
        .pin = DO_8_PIN,
    },

    {
        .port = (uint8_t*)DO_9_PORT,
        .pin = DO_9_PIN,
    },

    {
        .port = (uint8_t*)DO_10_PORT,
        .pin = DO_10_PIN,
    },

    {
        .port = (uint8_t*)DO_11_PORT,
        .pin = DO_11_PIN,
    },

    {
        .port = (uint8_t*)DO_12_PORT,
        .pin = DO_12_PIN,
    },

    {
        .port = (uint8_t*)DO_13_PORT,
        .pin = DO_13_PIN,
    },

    {
        .port = (uint8_t*)DO_14_PORT,
        .pin = DO_14_PIN,
    },

    {
        .port = (uint8_t*)DO_15_PORT,
        .pin = DO_15_PIN,
    },

    {
        .port = (uint8_t*)DO_16_PORT,
        .pin = DO_16_PIN,
    }
};