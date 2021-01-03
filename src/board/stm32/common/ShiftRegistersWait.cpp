/*

Copyright 2015-2021 Igor Petrovic

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

#include "board/Board.h"
#include "board/Internal.h"

//assuming 84MHz clock config

#define _NOP() __asm__ __volatile__("nop")

namespace Board
{
    namespace detail
    {
        namespace io
        {
            void sr595wait()
            {
                for (int i = 0; i < 12; i++)
                    _NOP();
            }

            void sr165wait()
            {
                for (int i = 0; i < 6; i++)
                    _NOP();
            }
        }    // namespace io
    }        // namespace detail
}    // namespace Board