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

class Viewtech : public IO::Touchscreen::Model, public IO::Touchscreen::Model::Common
{
    public:
    Viewtech(IO::Touchscreen::Model::HWA& hwa)
        : hwa(hwa)
    {}

    bool init() override;
    bool deInit() override;
    bool setScreen(size_t screenID) override;
    bool update(size_t& buttonID, bool& state) override;
    void setIconState(IO::Touchscreen::icon_t& icon, bool state) override;

    private:
    IO::Touchscreen::Model::HWA& hwa;

    const uint8_t endBytes   = 3;
    size_t        endCounter = 0;
};