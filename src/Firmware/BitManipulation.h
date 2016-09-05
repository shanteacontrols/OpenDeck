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