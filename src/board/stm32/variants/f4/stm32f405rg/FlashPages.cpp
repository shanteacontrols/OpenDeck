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

#define TOTAL_FLASH_PAGES 12

namespace
{
    Board::detail::map::flashPage_t pageDescriptor[TOTAL_FLASH_PAGES] = {
        {
            .address = 0x8000000,
            .size    = 16384,
        },

        {
            .address = 0x8004000,
            .size    = 16384,
        },

        {
            .address = 0x8008000,
            .size    = 16384,
        },

        {
            .address = 0x800C000,
            .size    = 16384,
        },

        {
            .address = 0x8010000,
            .size    = 65536,
        },

        //note: these pages are actually 128k pages (factory storage page, storage page 1 and storage page 2)
        //use only 64k so that caching of entire page is possible
        //first 128k page is used both for factory settings and part of firmware
        //place CDC firmware in lower 24kb range
        //for bootloader, use the real addresses and sizes so that firmware is updated properly
        //for app, use address with an offset (page 5 only) since app is only concerned with factory flash page and smaller sizes
        {
#ifdef FW_BOOT
            .address = 0x8020000,
            .size    = 131072,
#else
            .address = 0x8026000,
            .size    = 65536,
#endif
        },

        {
            .address = 0x8040000,
#ifdef FW_BOOT
            .size = 131072,
#else
            .size    = 65536,
#endif
        },

        {
            .address = 0x8060000,
#ifdef FW_BOOT
            .size = 131072,
#else
            .size    = 65536,
#endif
        },

        {
            .address = 0x8080000,
            .size    = 131072,
        },

        {
            .address = 0x80A0000,
            .size    = 131072,
        },

        {
            .address = 0x80C0000,
            .size    = 131072,
        },

        {
            .address = 0x80E0000,
            .size    = 131072,
        },
    };
}    // namespace

namespace Board
{
    namespace detail
    {
        namespace map
        {
            Board::detail::map::flashPage_t& flashPageDescriptor(size_t pageIndex)
            {
                if (pageIndex >= TOTAL_FLASH_PAGES)
                    return pageDescriptor[TOTAL_FLASH_PAGES - 1];

                return pageDescriptor[pageIndex];
            }

            size_t eepromFlashPageFactory()
            {
                return 5;
            }

            size_t eepromFlashPage1()
            {
                return 6;
            }

            size_t eepromFlashPage2()
            {
                return 7;
            }
        }    // namespace map
    }        // namespace detail
}    // namespace Board