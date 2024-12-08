/*

Copyright Igor Petrovic

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*/

#include "board.h"
#include "pins.h"

#include <Arduino.h>

void Board::setup()
{
    pinMode(SR_OUT_CLK_PIN, OUTPUT);
    pinMode(SR_OUT_LATCH_PIN, OUTPUT);
    pinMode(SR_OUT_DATA_PIN, OUTPUT);
    pinMode(SR_OUT_OE_PIN, OUTPUT);

    digitalWrite(SR_OUT_OE_PIN, LOW);

    pinMode(ANALOG_CTL_MEGA2560_1, OUTPUT);
    pinMode(ANALOG_CTL_MEGA2560_2, OUTPUT);
    pinMode(ANALOG_CTL_BLACKPILL_2, OUTPUT);
    pinMode(ANALOG_CTL_BLACKPILL_1, OUTPUT);
    pinMode(ANALOG_CTL_TEENSY_1, OUTPUT);
    pinMode(ANALOG_CTL_TEENSY_2, OUTPUT);
    pinMode(ANALOG_CTL_OD2_1, OUTPUT);
    pinMode(ANALOG_CTL_OD2_2, OUTPUT);

    pinMode(SR_IN_DATA_PIN, INPUT);
    pinMode(SR_IN_LATCH_PIN, OUTPUT);
    pinMode(SR_IN_CLK_PIN, OUTPUT);

    digitalWrite(SR_IN_LATCH_PIN, LOW);
    digitalWrite(SR_IN_CLK_PIN, LOW);

    digitalWrite(ANALOG_CTL_MEGA2560_1, LOW);
    digitalWrite(ANALOG_CTL_MEGA2560_2, LOW);
    digitalWrite(ANALOG_CTL_BLACKPILL_2, LOW);
    digitalWrite(ANALOG_CTL_BLACKPILL_1, LOW);
    digitalWrite(ANALOG_CTL_TEENSY_1, LOW);
    digitalWrite(ANALOG_CTL_TEENSY_2, LOW);
    digitalWrite(ANALOG_CTL_OD2_1, LOW);
    digitalWrite(ANALOG_CTL_OD2_2, LOW);

    write(output_t::pwr, true);
}

void Board::updateOutSR(uint8_t output, bool state)
{
    if (output >= 8)
    {
        return;
    }

    bitWrite(outSrState, output, state);

    digitalWrite(SR_OUT_LATCH_PIN, LOW);

    for (int i = 0; i < 8; i++)
    {
        digitalWrite(SR_OUT_DATA_PIN, bitRead(outSrState, i));
        digitalWrite(SR_OUT_CLK_PIN, LOW);
        delay(1);
        digitalWrite(SR_OUT_CLK_PIN, HIGH);
    }

    digitalWrite(SR_OUT_LATCH_PIN, HIGH);
}

void Board::updateInSR()
{
    digitalWrite(SR_IN_CLK_PIN, LOW);
    digitalWrite(SR_IN_LATCH_PIN, LOW);
    delay(1);
    digitalWrite(SR_IN_LATCH_PIN, HIGH);

    for (int input = 0; input < 8; input++)
    {
        digitalWrite(SR_IN_CLK_PIN, LOW);
        delay(1);
        bitWrite(inSrReadings, input, digitalRead(SR_IN_DATA_PIN));
        digitalWrite(SR_IN_CLK_PIN, HIGH);
    }
}
