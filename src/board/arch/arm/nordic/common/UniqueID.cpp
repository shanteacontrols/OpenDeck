/*

Copyright 2015-2022 Igor Petrovic

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
#include "nrf.h"

#ifdef USB_SUPPORTED
namespace Board
{
    void uniqueID(uniqueID_t& uid)
    {
        uint32_t id[2];

        id[0] = NRF_FICR->DEVICEADDR[0];
        id[1] = NRF_FICR->DEVICEADDR[1];

        for (int i = 0; i < 2; i++)
        {
            for (int j = 0; j < 4; j++)
                uid[(i * 4) + j] = id[i] >> ((3 - j) * 8) & 0xFF;
        }
    }
}    // namespace Board
#endif