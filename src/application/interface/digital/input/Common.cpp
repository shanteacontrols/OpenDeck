/*

Copyright 2015-2019 Igor Petrovic

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

namespace Interface
{
    namespace digital
    {
        namespace input
        {
            uint8_t Common::pcValue[16] = {};

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
        }    // namespace input
    }        // namespace digital
}    // namespace Interface