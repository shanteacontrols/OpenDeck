/*

Copyright 2015-2020 Igor Petrovic

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

namespace Board
{
    namespace detail
    {
        namespace map
        {
            uint32_t adcChannel(core::io::mcuPin_t pin)
            {
                if (&pin.port == PORTF)
                {
                    if ((pin.index != 2) && (pin.index != 3))
                        return pin.index;
                }
                else if (&pin.port == PORTD)
                {
                    switch (pin.index)
                    {
                    case 4:
                        return 8;

                    case 6:
                        return 9;

                    case 7:
                        return 10;

                    default:
                        return 0xFF;
                    }
                }
                else if (&pin.port == PORTB)
                {
                    switch (pin.index)
                    {
                    case 5:
                        return 12;

                    case 6:
                        return 13;

                    default:
                        return 0xFF;
                    }
                }

                return 0xFF;
            }
        }    // namespace map
    }        // namespace detail
}    // namespace Board