/*

Copyright Igor Petrovic

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

#include "common.h"

#include <stddef.h>

namespace updater
{
    class Hwa
    {
        public:
        virtual uint32_t pageSize(size_t index)                                   = 0;
        virtual void     erasePage(size_t index)                                  = 0;
        virtual void     fillPage(size_t index, uint32_t address, uint32_t value) = 0;
        virtual void     commitPage(size_t index)                                 = 0;
        virtual void     apply()                                                  = 0;
        virtual void     onFirmwareUpdateStart()                                  = 0;
    };
}    // namespace updater
