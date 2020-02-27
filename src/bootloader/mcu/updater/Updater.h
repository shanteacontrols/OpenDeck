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

#pragma once

#include <inttypes.h>
#include "IBTLDRWriter.h"

namespace Bootloader
{
    class Updater
    {
        public:
        Updater(IBTLDRWriter& writer, bool dwordAddress, const uint32_t pageSize, const uint32_t updateCompletePage)
            : writer(writer)
            , addressBytes(dwordAddress ? 4 : 2)
            , pageSize(pageSize)
            , updateCompletePage(dwordAddress ? updateCompletePage : updateCompletePage & 0xFFFF)
        {}

        void feed(uint16_t data);
        void reset();

        private:
        uint8_t        pageBytesReceived = 0;
        uint32_t       currentPage       = 0;
        uint32_t       pageWord          = 0;
        IBTLDRWriter&  writer;
        const uint8_t  addressBytes;
        const uint32_t pageSize;
        const uint32_t updateCompletePage;
    };
}    // namespace Bootloader