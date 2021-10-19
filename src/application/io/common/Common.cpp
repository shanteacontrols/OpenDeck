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

#include "Common.h"
#include "core/src/general/Helpers.h"

namespace
{
    uint8_t pcValue[16];
    uint8_t midiValue[MAX_NUMBER_OF_BUTTONS + MAX_NUMBER_OF_ANALOG + MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS];
}    // namespace

namespace IO
{
    bool Common::pcIncrement(uint8_t channel)
    {
        if (channel >= 16)
            return false;

        if (pcValue[channel] < 127)
        {
            pcValue[channel]++;
            return true;
        }

        return false;
    }

    bool Common::pcDecrement(uint8_t channel)
    {
        if (channel >= 16)
            return false;

        if (pcValue[channel] > 0)
        {
            pcValue[channel]--;
            return true;
        }

        return false;
    }

    uint8_t Common::program(uint8_t channel)
    {
        if (channel >= 16)
            return false;

        return pcValue[channel];
    }

    bool Common::setProgram(uint8_t channel, uint8_t program)
    {
        if (channel >= 16)
            return false;

        if (program > 127)
            return false;

        pcValue[channel] = program;
        return true;
    }

    uint8_t Common::valueInc(size_t index, uint8_t step, incDecType_t type)
    {
        if (index >= (MAX_NUMBER_OF_BUTTONS + MAX_NUMBER_OF_ANALOG + MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS))
            return 0xFF;

        step &= 0x7F;    // safety
        uint8_t newValue    = (midiValue[index] & 0x7F) + step;
        bool    setNewValue = true;

        switch (type)
        {
        case incDecType_t::toEdge:
        {
            // just make sure the value isn't larger than 127

            if (newValue > 127)
                newValue = 127;
        }
        break;

        case incDecType_t::reset:
        {
            // in case value is larger than 127, send 127 but don't apply it internally
            // this way we're starting from 0 next time
            if (newValue > 127)
            {
                newValue    = 127;
                setNewValue = false;
            }
        }
        break;

        default:
            break;
        }

        // make sure increment/decrement flag is preserved
        midiValue[index] &= 0x80;

        if (setNewValue)
            midiValue[index] |= newValue;

        return newValue;
    }

    uint8_t Common::valueIncDec(size_t index, uint8_t step)
    {
        if (index >= (MAX_NUMBER_OF_BUTTONS + MAX_NUMBER_OF_ANALOG + MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS))
            return 0xFF;

        uint8_t newValue = 0xFF;

        // in this case, once the value is larger than 127, decrement value
        // bit 7 in midiValue is used as an indicator that we should decrement the value instead of incrementing it
        if (BIT_READ(midiValue[index], 7))
        {
            if ((midiValue[index] & 0x7F) <= step)
                newValue = 0;
            else
                newValue = (midiValue[index] & 0x7F) - step;

            // value 0 is reached, go back to incrementing next time
            if (!newValue)
                BIT_CLEAR(midiValue[index], 7);
        }
        else
        {
            newValue = (midiValue[index] & 0x7F) + step;

            if (newValue > 127)
            {
                newValue = 127;
                BIT_SET(midiValue[index], 7);
            }
        }

        // make sure increment/decrement flag is preserved
        midiValue[index] &= 0x80;
        midiValue[index] |= newValue;

        return newValue;
    }

    uint8_t Common::currentValue(size_t index)
    {
        if (index >= (MAX_NUMBER_OF_BUTTONS + MAX_NUMBER_OF_ANALOG + MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS))
            return 0xFF;

        return midiValue[index] & 0x7F;
    }

    void Common::resetValue(size_t index)
    {
        midiValue[index] = 0;
    }
}    // namespace IO