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

#pragma once

#include <inttypes.h>
#include <stdlib.h>

namespace Bootloader
{
    class Updater
    {
        public:
        class BTLDRWriter
        {
            public:
            virtual size_t pageSize(size_t index)                                  = 0;
            virtual void   erasePage(size_t index)                                 = 0;
            virtual void   fillPage(size_t index, uint32_t address, uint16_t data) = 0;
            virtual void   writePage(size_t index)                                 = 0;
            virtual void   apply()                                                 = 0;
        };

        Updater(BTLDRWriter& writer, const uint32_t startValue, uint32_t endValue)
            : writer(writer)
            , startValue(startValue)
            , endValue(endValue)
        {}

        void feed(uint8_t data);
        void reset();

        private:
        enum class receiveStage_t : uint8_t
        {
            start,
            fwMetadata,
            fwChunk,
            end
        };

        bool processStart(uint8_t data);
        bool processFwMetadata(uint8_t data);
        bool processFwChunk(uint8_t data);
        bool processEnd(uint8_t data);

        receiveStage_t currentStage      = receiveStage_t::start;
        size_t         currentPage       = 0;
        uint16_t       receivedWord      = 0;
        size_t         pageBytesReceived = 0;
        uint32_t       fwBytesReceived   = 0;
        uint32_t       fwSize            = 0;
        uint8_t        byteCountReceived = 0;
        BTLDRWriter&   writer;
        const uint32_t startValue;
        const uint32_t endValue;
    };
}    // namespace Bootloader