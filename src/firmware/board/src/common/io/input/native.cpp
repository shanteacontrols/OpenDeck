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

#ifdef PROJECT_TARGET_SUPPORT_DIGITAL_INPUTS
#ifdef PROJECT_TARGET_DRIVER_DIGITAL_INPUT_NATIVE

#include "board/board.h"
#include "internal.h"
#include <target.h>

#include "core/util/util.h"
#include "core/util/ring_buffer.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

using namespace board::io::digital_in;
using namespace board::detail;
using namespace board::detail::io::digital_in;

namespace
{
    volatile Readings                                                     digitalInBuffer[PROJECT_TARGET_MAX_NR_OF_DIGITAL_INPUTS];
    core::util::RingBuffer<core::mcu::io::portWidth_t, MAX_READING_COUNT> portBuffer[PROJECT_TARGET_NR_OF_DIGITAL_INPUT_PORTS];

    inline void storeDigitalIn()
    {
        // read all input ports instead of reading pin by pin to reduce the time spent in ISR
        for (uint8_t portIndex = 0; portIndex < PROJECT_TARGET_NR_OF_DIGITAL_INPUT_PORTS; portIndex++)
        {
            portBuffer[portIndex].insert(CORE_MCU_IO_READ_IN_PORT(map::DIGITAL_IN_PORT(portIndex)));
        }
    }

    inline void fillBuffer(size_t digitalInIndex)
    {
        // for provided button index, retrieve its port index
        // upon reading update all buttons located on that port

        auto                       portIndex = map::BUTTON_PORT_INDEX(digitalInIndex);
        core::mcu::io::portWidth_t portValue = 0;

        while (portBuffer[portIndex].remove(portValue))
        {
            for (size_t i = 0; i < PROJECT_TARGET_MAX_NR_OF_DIGITAL_INPUTS; i++)
            {
                if (map::BUTTON_PORT_INDEX(i) == portIndex)
                {
                    digitalInBuffer[i].readings <<= 1;
                    digitalInBuffer[i].readings |= !core::util::BIT_READ(portValue, map::BUTTON_PIN_INDEX(i));

                    if (++digitalInBuffer[i].count > MAX_READING_COUNT)
                    {
                        digitalInBuffer[i].count = MAX_READING_COUNT;
                    }
                }
            }
        }
    }
}    // namespace

namespace board::detail::io::digital_in
{
    void init()
    {
        for (size_t i = 0; i < PROJECT_TARGET_MAX_NR_OF_DIGITAL_INPUTS; i++)
        {
            auto pin = detail::map::BUTTON_PIN(i);

#ifndef PROJECT_TARGET_BUTTONS_EXT_PULLUPS
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
}    // namespace board::detail::io::digital_in

namespace board::io::digital_in
{
    bool state(size_t index, Readings& readings)
    {
        if (index >= PROJECT_TARGET_MAX_NR_OF_DIGITAL_INPUTS)
        {
            return false;
        }

        fillBuffer(index);

        index                        = map::BUTTON_INDEX(index);
        readings.count               = digitalInBuffer[index].count;
        readings.readings            = digitalInBuffer[index].readings;
        digitalInBuffer[index].count = 0;

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
}    // namespace board::io::digital_in

#include "common.cpp.include"

#endif
#endif