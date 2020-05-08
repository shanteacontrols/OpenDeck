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
#include "core/src/general/Reset.h"
#include "core/src/general/Interrupt.h"

namespace Board
{
    bool checkNewRevision()
    {
        return false;
    }

    void uniqueID(uniqueID_t& uid)
    {
        uint32_t id[3];

        id[0] = HAL_GetUIDw0();
        id[1] = HAL_GetUIDw1();
        id[2] = HAL_GetUIDw2();

        for (int i = 0; i < 3; i++)
        {
            for (int j = 0; j < 4; j++)
                uid.uid[(i * 4) + j] = id[i] >> ((3 - j) * 8) & 0xFF;
        }
    }

    namespace detail
    {
        namespace setup
        {
            void application()
            {
                //Reset of all peripherals, Initializes the Flash interface and the Systick
                HAL_Init();

                detail::setup::clocks();
                detail::setup::io();

                eeprom::init();
                detail::setup::adc();

                detail::setup::timers();

#ifdef USB_MIDI_SUPPORTED
                detail::setup::usb();
#endif
            }

            void bootloader()
            {
                HAL_Init();

                detail::setup::clocks();
                detail::setup::io();
            }
        }    // namespace setup
    }        // namespace detail
}    // namespace Board