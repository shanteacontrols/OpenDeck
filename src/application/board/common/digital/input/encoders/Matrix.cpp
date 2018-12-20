/*

Copyright 2015-2018 Igor Petrovic

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

#include "Common.h"
#include "board/common/digital/input/Variables.h"

namespace Board
{
    uint8_t getEncoderPair(uint8_t buttonID)
    {
        uint8_t row = buttonID/NUMBER_OF_BUTTON_COLUMNS;
        uint8_t column = buttonID % NUMBER_OF_BUTTON_COLUMNS;

        if (row%2)
            row -= 1;   //uneven row, get info from previous (even) row

        return (row*NUMBER_OF_BUTTON_COLUMNS)/2 + column;
    }

    encoderPosition_t getEncoderState(uint8_t encoderID, uint8_t pulsesPerStep)
    {
        using namespace Board::detail;

        uint8_t column = encoderID % NUMBER_OF_BUTTON_COLUMNS;
        uint8_t row  = (encoderID/NUMBER_OF_BUTTON_COLUMNS)*2;
        uint8_t pairState = (digitalInBufferReadOnly[column] >> row) & 0x03;

        return readEncoder(encoderID, pairState, pulsesPerStep);
    }
}