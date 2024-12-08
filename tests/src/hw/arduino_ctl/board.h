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

#pragma once

#include <inttypes.h>

class Board
{
    public:
    Board() = default;

    enum class input_t : uint8_t
    {
        out1_t2pp,
        out2_t2pp,
        out1_bp,
        out2_bp,
        out1_od2,
        out2_od2,
        out1_mega,
        out2_mega,
    };

    enum class output_t : uint8_t
    {
        pwr          = 1,
        sw2_avr      = 2,
        sw1_avr      = 3,
        sw1_bp       = 4,
        sw2_bp       = 5,
        sw2_od2      = 6,
        sw1_od2      = 7,
        analog1_od2  = 8,
        analog2_od2  = 9,
        analog1_bp   = 10,
        analog2_bp   = 11,
        analog1_mega = 12,
        analog2_mega = 13,
        analog1_t2pp = 14,
        analog2_t2pp = 15,
    };

    void setup();

    template<typename T>
    bool read(T pin)
    {
        auto pinCast = static_cast<uint8_t>(pin);

        if (pinCast < 8)
        {
            updateInSR();
            return bitRead(inSrReadings, pin);
        }
        else
        {
            pinCast -= 8;
            return digitalRead(pinCast);
        }

        return false;
    }

    template<typename T>
    void write(T pin, bool state)
    {
        auto pinCast = static_cast<uint8_t>(pin);

        if (pinCast < 8)
        {
            updateOutSR(pinCast, state);
        }
        else
        {
            pinCast -= 8;
            digitalWrite(pinCast, state);
        }
    }

    private:
    void updateOutSR(uint8_t output, bool state);
    void updateInSR();

    uint8_t inSrReadings = 0;
    uint8_t outSrState   = 0;
};
