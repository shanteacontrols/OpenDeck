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

namespace fw_selector
{
    class Hwa
    {
        public:
        virtual uint32_t magicBootValue()                                 = 0;
        virtual void     setMagicBootValue(uint32_t value)                = 0;
        virtual void     load(fwType_t fwType)                            = 0;
        virtual void     appAddrBoundary(uint32_t& first, uint32_t& last) = 0;
        virtual bool     isHwTriggerActive()                              = 0;
        virtual uint8_t  readFlash(uint32_t address)                      = 0;
    };
}    // namespace fw_selector
