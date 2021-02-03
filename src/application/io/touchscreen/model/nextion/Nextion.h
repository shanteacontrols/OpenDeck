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

#pragma once

#include <inttypes.h>
#include "io/touchscreen/Touchscreen.h"
#include "core/src/general/RingBuffer.h"

class Nextion : public IO::Touchscreen::Model, public IO::Touchscreen::Model::Common
{
    public:
    Nextion(IO::Touchscreen::Model::HWA& hwa)
        : hwa(hwa)
    {}

    bool                       init() override;
    bool                       deInit() override;
    bool                       setScreen(size_t screenID) override;
    IO::Touchscreen::tsEvent_t update(IO::Touchscreen::tsData_t& data) override;
    void                       setIconState(IO::Touchscreen::icon_t& icon, bool state) override;
    bool                       setBrightness(IO::Touchscreen::brightness_t brightness) override;

    private:
    bool writeCommand(const char* line, ...);
    bool endCommand();

    IO::Touchscreen::Model::HWA& hwa;

    //there are 7 levels of brighness - scale them to available range (0-100)
    const uint8_t brightnessMapping[7] = {
        10,
        25,
        50,
        75,
        80,
        90,
        100
    };

    char   commandBuffer[IO::Touchscreen::Model::Common::bufferSize];
    size_t endCounter = 0;
};