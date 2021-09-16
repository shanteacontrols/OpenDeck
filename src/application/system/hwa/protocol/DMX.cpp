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

bool System::HWADMX::init()
{
    return _system._hwa.protocol().dmx().init();
}

bool System::HWADMX::deInit()
{
    return _system._hwa.protocol().dmx().deInit();
}

bool System::HWADMX::readUSB(uint8_t* buffer, size_t& size, const size_t maxSize)
{
    return _system._hwa.protocol().dmx().readUSB(buffer, size, maxSize);
}

bool System::HWADMX::writeUSB(uint8_t* buffer, size_t size)
{
    return _system._hwa.protocol().dmx().writeUSB(buffer, size);
}

bool System::HWADMX::updateChannel(uint16_t channel, uint8_t value)
{
    return _system._hwa.protocol().dmx().updateChannel(channel, value);
}