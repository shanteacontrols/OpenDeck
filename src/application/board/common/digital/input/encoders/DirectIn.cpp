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
#include "../Variables.h"
#include "core/src/general/BitManipulation.h"
#include "board/Board.h"

namespace Board
{
    uint8_t getEncoderPair(uint8_t buttonID)
    {
        return buttonID/2;
    }

    encoderPosition_t getEncoderState(uint8_t encoderID, uint8_t pulsesPerStep)
    {
        using namespace Board::detail;

        uint8_t buttonID = encoderID*2;

        uint8_t pairState = Board::getButtonState(buttonID);
        pairState <<= 1;
        pairState |= Board::getButtonState(buttonID+1);

        return readEncoder(encoderID, pairState, pulsesPerStep);
    }
}