/*

Copyright 2015-2022 Igor Petrovic

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
#include "core/src/util/RingBuffer.h"

class Nextion : public IO::Touchscreen::Model
{
    public:
    Nextion(IO::Touchscreen::HWA& hwa);

    bool                       init() override;
    bool                       deInit() override;
    bool                       setScreen(size_t screenID) override;
    IO::Touchscreen::tsEvent_t update(IO::Touchscreen::tsData_t& data) override;
    void                       setIconState(IO::Touchscreen::icon_t& icon, bool state) override;
    bool                       setBrightness(IO::Touchscreen::brightness_t brightness) override;

    private:
    enum class responseID_t : uint8_t
    {
        BUTTON,
        AMOUNT
    };

    struct responseDescriptor_t
    {
        uint8_t size;
        uint8_t responseID;
    };

    bool                       writeCommand(const char* line, ...);
    bool                       endCommand();
    IO::Touchscreen::tsEvent_t response(IO::Touchscreen::tsData_t& data);

    IO::Touchscreen::HWA& _hwa;

    char   _commandBuffer[IO::Touchscreen::Model::BUFFER_SIZE];
    size_t _endCounter = 0;

    static constexpr responseDescriptor_t RESPONSES[static_cast<size_t>(responseID_t::AMOUNT)] = {
        // button
        {
            .size       = 6,
            .responseID = 0x65,
        },
    };

    // there are 7 levels of brighness - scale them to available range (0-100)
    static constexpr uint8_t BRIGHTNESS_MAPPING[7] = {
        10,
        25,
        50,
        75,
        80,
        90,
        100
    };
};