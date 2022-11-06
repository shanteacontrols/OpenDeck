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

#ifdef HW_SUPPORT_DIGITAL_INPUTS
#ifdef HW_DRIVER_DIGITAL_INPUT_NATIVE

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "board/Board.h"
#include "board/Internal.h"
#include "core/src/util/Util.h"
#include "core/src/util/RingBuffer.h"
#include <Target.h>

using namespace board::io::digitalIn;
using namespace board::detail;
using namespace board::detail::io::digitalIn;

namespace
{
    volatile readings_t                                                   _digitalInBuffer[HW_MAX_NR_OF_DIGITAL_INPUTS];
    core::util::RingBuffer<core::mcu::io::portWidth_t, MAX_READING_COUNT> _portBuffer[HW_NR_OF_DIGITAL_INPUT_PORTS];

    inline void storeDigitalIn()
    {
        // read all input ports instead of reading pin by pin to reduce the time spent in ISR
        for (uint8_t portIndex = 0; portIndex < HW_NR_OF_DIGITAL_INPUT_PORTS; portIndex++)
        {
            _portBuffer[portIndex].insert(CORE_MCU_IO_READ_IN_PORT(map::DIGITAL_IN_PORT(portIndex)));
        }
    }

    inline void fillBuffer(size_t digitalInIndex)
    {
        // for provided button index, retrieve its port index
        // upon reading update all buttons located on that port

        auto                       portIndex = map::BUTTON_PORT_INDEX(digitalInIndex);
        core::mcu::io::portWidth_t portValue = 0;

        while (_portBuffer[portIndex].remove(portValue))
        {
            for (size_t i = 0; i < HW_MAX_NR_OF_DIGITAL_INPUTS; i++)
            {
                if (map::BUTTON_PORT_INDEX(i) == portIndex)
                {
                    _digitalInBuffer[i].readings <<= 1;
                    _digitalInBuffer[i].readings |= !core::util::BIT_READ(portValue, map::BUTTON_PIN_INDEX(i));

                    if (++_digitalInBuffer[i].count > MAX_READING_COUNT)
                    {
                        _digitalInBuffer[i].count = MAX_READING_COUNT;
                    }
                }
            }
        }
    }
}    // namespace

namespace board::detail::io::digitalIn
{
    void init()
    {
        for (size_t i = 0; i < HW_MAX_NR_OF_DIGITAL_INPUTS; i++)
        {
            auto pin = detail::map::BUTTON_PIN(i);

#ifndef HW_BUTTONS_EXT_PULLUPS
            CORE_MCU_IO_INIT(pin.port,
                             pin.index,
                             core::mcu::io::pinMode_t::INPUT,
                             core::mcu::io::pullMode_t::UP);
#else
            CORE_MCU_IO_INIT(pin.port,
                             pin.index,
                             core::mcu::io::pinMode_t::INPUT,
                             core::mcu::io::pullMode_t::NONE);
#endif
        }
    }
}    // namespace board::detail::io::digitalIn

namespace board::io::digitalIn
{
    bool state(size_t index, readings_t& readings)
    {
        if (index >= HW_MAX_NR_OF_DIGITAL_INPUTS)
        {
            return false;
        }

        fillBuffer(index);

        index                         = map::BUTTON_INDEX(index);
        readings.count                = _digitalInBuffer[index].count;
        readings.readings             = _digitalInBuffer[index].readings;
        _digitalInBuffer[index].count = 0;

        return readings.count > 0;
    }

    size_t encoderFromInput(size_t index)
    {
        return index / 2;
    }

    size_t encoderComponentFromEncoder(size_t index, encoderComponent_t component)
    {
        index *= 2;

        if (component == encoderComponent_t::A)
        {
            return index;
        }

        return index + 1;
    }
}    // namespace board::io::digitalIn

#include "Common.cpp.include"

#endif
#endif