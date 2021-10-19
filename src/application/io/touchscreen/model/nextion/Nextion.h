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
#include "io/touchscreen/model/Base.h"
#include "core/src/general/RingBuffer.h"

class Nextion : public IO::TouchscreenBase, public IO::TouchscreenBase::Common
{
    public:
    Nextion(IO::TouchscreenBase::HWA& hwa)
        : _hwa(hwa)
    {}

    bool                           init() override;
    bool                           deInit() override;
    bool                           setScreen(size_t screenID) override;
    IO::TouchscreenBase::tsEvent_t update(IO::TouchscreenBase::tsData_t& data) override;
    void                           setIconState(IO::TouchscreenBase::icon_t& icon, bool state) override;
    bool                           setBrightness(IO::TouchscreenBase::brightness_t brightness) override;

    private:
    enum class responseID_t : uint8_t
    {
        button,
        initialFinalCoord,
        coordUpdate,
        AMOUNT
    };

    typedef struct
    {
        uint8_t size;
        uint8_t responseID;
    } responseDescriptor_t;

    enum class xyRequestState_t : uint8_t
    {
        xRequest,
        xRequested,
        yRequest,
        yRequested,
    };

    bool                           writeCommand(const char* line, ...);
    bool                           endCommand();
    IO::TouchscreenBase::tsEvent_t response(IO::TouchscreenBase::tsData_t& data);
    void                           pollXY();

    static constexpr uint32_t XY_POLL_TIME_MS = 5;

    IO::TouchscreenBase::HWA& _hwa;

    char             _commandBuffer[IO::TouchscreenBase::Common::bufferSize];
    size_t           _endCounter     = 0;
    bool             _screenPressed  = false;
    xyRequestState_t _xyRequestState = xyRequestState_t::xRequest;
    uint16_t         _xPos           = 0;

    const responseDescriptor_t _responses[static_cast<size_t>(responseID_t::AMOUNT)] = {
        // button
        {
            .size       = 6,
            .responseID = 0x65,
        },

        // coordinate initial/final
        {
            .size       = 9,
            .responseID = 0x67,
        },

        // coordinate update
        {
            .size       = 8,
            .responseID = 0x71,
        },
    };

    // there are 7 levels of brighness - scale them to available range (0-100)
    const uint8_t _brightnessMapping[7] = {
        10,
        25,
        50,
        75,
        80,
        90,
        100
    };
};