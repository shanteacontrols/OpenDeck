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

void System::HWALEDs::setState(size_t index, IO::LEDs::brightness_t brightness)
{
#if MAX_NUMBER_OF_LEDS > 0
#if MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS != 0
    if (index >= MAX_NUMBER_OF_LEDS)
        _system._touchscreen.setIconState(index - MAX_NUMBER_OF_LEDS, brightness != IO::LEDs::brightness_t::bOff);
    else
        _system._hwa.io().leds().setState(index, brightness);
#else
    _system._hwa.io().leds().setState(index, brightness);
#endif
#else
    _system._touchscreen.setIconState(index, brightness != IO::LEDs::brightness_t::bOff);
#endif
}

size_t System::HWALEDs::rgbIndex(size_t singleLEDindex)
{
    return _system._hwa.io().leds().rgbIndex(singleLEDindex);
}

size_t System::HWALEDs::rgbSignalIndex(size_t rgbIndex, IO::LEDs::rgbIndex_t rgbComponent)
{
    return _system._hwa.io().leds().rgbSignalIndex(rgbIndex, rgbComponent);
}