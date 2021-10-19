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

bool System::HWATouchscreen::init()
{
    if (_system._hwa.io().touchscreen().init())
    {
        // add slight delay before display becomes ready on power on
        core::timing::waitMs(1000);

        return true;
    }

    return false;
}

bool System::HWATouchscreen::deInit()
{
    return _system._hwa.io().touchscreen().deInit();
}

bool System::HWATouchscreen::write(uint8_t value)
{
    return _system._hwa.io().touchscreen().write(value);
}

bool System::HWATouchscreen::read(uint8_t& value)
{
    return _system._hwa.io().touchscreen().read(value);
}