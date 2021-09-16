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

#include "system/System.h"

bool System::HWACDCPassthrough::init()
{
    return _system._hwa.io().cdcPassthrough().init();
}

bool System::HWACDCPassthrough::deInit()
{
    return _system._hwa.io().cdcPassthrough().deInit();
}

bool System::HWACDCPassthrough::uartRead(uint8_t& value)
{
    return _system._hwa.io().cdcPassthrough().uartRead(value);
}

bool System::HWACDCPassthrough::uartWrite(uint8_t value)
{
    return _system._hwa.io().cdcPassthrough().uartWrite(value);
}

bool System::HWACDCPassthrough::cdcRead(uint8_t* buffer, size_t& size, const size_t maxSize)
{
    return _system._hwa.io().cdcPassthrough().cdcRead(buffer, size, maxSize);
}

bool System::HWACDCPassthrough::cdcWrite(uint8_t* buffer, size_t size)
{
    return _system._hwa.io().cdcPassthrough().cdcWrite(buffer, size);
}