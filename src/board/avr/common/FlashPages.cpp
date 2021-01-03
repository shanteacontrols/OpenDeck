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

#include <avr/io.h>
#include "board/Board.h"
#include "board/Internal.h"

namespace
{
    Board::detail::map::flashPage_t pageDescriptor = {
        .address = 0,
        .size    = SPM_PAGESIZE,
    };
}

namespace Board
{
    namespace detail
    {
        namespace map
        {
            Board::detail::map::flashPage_t& flashPageDescriptor(size_t pageIndex)
            {
                //all flash pages on avr have the same size
                //calculate address only

                uint32_t address = pageIndex * SPM_PAGESIZE;

                if (address > FLASHEND)
                    pageDescriptor.address = 0;
                else
                    pageDescriptor.address = address;

                return pageDescriptor;
            }
        }    // namespace map
    }        // namespace detail
}    // namespace Board