/*
    OpenDeck MIDI platform firmware
    Copyright (C) 2015, 2016 Igor Petrovic

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

#ifndef BITMANIPULATION_H_
#define BITMANIPULATION_H_

#include <stdio.h>

#define bitRead(value, bit) (((value) >> (bit)) & 0x01)
#define bitSet(value, bit) ((value) |= (1UL << (bit)))
#define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
#define bitWrite(value, bit, bitvalue) (bitvalue ? bitSet(value, bit) : bitClear(value, bit))
#define invertByte(value) ((value) ^ 0xFF)
#define lowByte_7bit(value) ((value) & 0x7F)
#define highByte_7bit(value) ((value >> 7) & 0x7f)

typedef struct {

    uint8_t high;
    uint8_t low;
    uint16_t value;

    void encodeTo14bit()    {

        uint8_t newHigh = (value >> 8) & 0xFF;
        uint8_t newLow = value & 0xFF;
        newHigh = (newHigh << 1) & 0x7F;
        bitWrite(newHigh, 0, bitRead(newLow, 7));
        newLow = lowByte_7bit(newLow);
        high = newHigh;
        low = newLow;

    }

    uint16_t decode14bit()  {

        bitWrite(low, 7, bitRead(high, 0));
        high >>= 1;

        uint16_t joined;
        joined = high;
        joined <<= 8;
        joined |= low;

        return joined;

    }

} encDec_14bit;

#endif