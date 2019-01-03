/*

Copyright 2015-2019 Igor Petrovic

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

#include "../Variables.h"
#include "core/src/general/BitManipulation.h"

namespace Board
{
    bool getButtonState(uint8_t buttonID)
    {
        using namespace Board::detail;

        uint8_t arrayIndex = buttonID/8;
        uint8_t buttonIndex = buttonID - 8*arrayIndex;

        return BIT_READ(digitalInBufferReadOnly[arrayIndex], buttonIndex);
    }
}