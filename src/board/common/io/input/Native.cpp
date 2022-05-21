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

#ifdef DIGITAL_INPUTS_SUPPORTED
#ifdef DIGITAL_INPUT_DRIVER_NATIVE

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "board/Board.h"
#include "board/Internal.h"
#include "board/common/constants/IO.h"
#include "core/src/util/Util.h"
#include "core/src/util/RingBuffer.h"
#include <Target.h>

using namespace Board::detail;

namespace
{
    volatile Board::IO::dInReadings_t                                         _digitalInBuffer[NR_OF_DIGITAL_INPUTS];
    core::util::RingBuffer<core::mcu::io::portWidth_t, IO::MAX_READING_COUNT> _portBuffer[NR_OF_DIGITAL_INPUT_PORTS];

    inline void storeDigitalIn()
    {
        // read all input ports instead of reading pin by pin to reduce the time spent in ISR
        for (int portIndex = 0; portIndex < NR_OF_DIGITAL_INPUT_PORTS; portIndex++)
        {
            _portBuffer[portIndex].insert(CORE_MCU_IO_READ_IN_PORT(map::digitalInPort(portIndex)));
        }
    }

    inline void fillBuffer(size_t digitalInIndex)
    {
        // for provided button index, retrieve its port index
        // upon reading update all buttons located on that port

        auto                       portIndex = map::buttonPortIndex(digitalInIndex);
        core::mcu::io::portWidth_t portValue = 0;

        while (_portBuffer[portIndex].remove(portValue))
        {
            for (int i = 0; i < NR_OF_DIGITAL_INPUTS; i++)
            {
                if (map::buttonPortIndex(i) == portIndex)
                {
                    _digitalInBuffer[i].readings <<= 1;
                    _digitalInBuffer[i].readings |= !core::util::BIT_READ(portValue, map::buttonPinIndex(i));

                    if (++_digitalInBuffer[i].count > IO::MAX_READING_COUNT)
                    {
                        _digitalInBuffer[i].count = IO::MAX_READING_COUNT;
                    }
                }
            }
        }
    }
}    // namespace

namespace Board::IO
{
    bool digitalInState(size_t digitalInIndex, dInReadings_t& dInReadings)
    {
        if (digitalInIndex >= NR_OF_DIGITAL_INPUTS)
        {
            return false;
        }

        fillBuffer(digitalInIndex);

        digitalInIndex                         = detail::map::buttonIndex(digitalInIndex);
        dInReadings.count                      = _digitalInBuffer[digitalInIndex].count;
        dInReadings.readings                   = _digitalInBuffer[digitalInIndex].readings;
        _digitalInBuffer[digitalInIndex].count = 0;

        return dInReadings.count > 0;
    }

    size_t encoderIndex(size_t buttonID)
    {
        return buttonID / 2;
    }

    size_t encoderSignalIndex(size_t encoderID, encoderIndex_t index)
    {
        uint8_t buttonID = encoderID * 2;

        if (index == encoderIndex_t::A)
        {
            return buttonID;
        }

        return buttonID + 1;
    }
}    // namespace Board::IO

#include "Common.cpp.include"

#endif
#endif