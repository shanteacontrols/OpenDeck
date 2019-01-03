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

#include "board/common/digital/input/Variables.h"
#include "core/src/general/BitManipulation.h"

namespace Board
{
    bool getButtonState(uint8_t buttonIndex)
    {
        using namespace Board::detail;

        uint8_t row = buttonIndex/NUMBER_OF_BUTTON_COLUMNS;
        uint8_t column = buttonIndex % NUMBER_OF_BUTTON_COLUMNS;

        return BIT_READ(digitalInBufferReadOnly[column], row);
    }
}