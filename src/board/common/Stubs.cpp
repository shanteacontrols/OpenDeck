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

#include "board/Board.h"
#include "board/Internal.h"

// Stub functions are used so that firmware can be compiled without
// resorting to ifdef mess if something isn't supported

namespace Board
{
    __attribute__((weak)) void update()
    {
    }

    namespace IO
    {
        namespace digitalIn
        {
            __attribute__((weak)) bool state(size_t index, readings_t& readings)
            {
                return false;
            }

            __attribute__((weak)) size_t encoderFromInput(size_t index)
            {
                return 0;
            }

            __attribute__((weak)) size_t encoderComponentFromEncoder(size_t index, encoderComponent_t component)
            {
                return 0;
            }
        }    // namespace digitalIn

        namespace digitalOut
        {
            __attribute__((weak)) void writeLEDstate(size_t index, ledBrightness_t ledBrightness)
            {
            }

            __attribute__((weak)) size_t rgbFromOutput(size_t index)
            {
                return 0;
            }

            __attribute__((weak)) size_t rgbComponentFromRGB(size_t index, rgbComponent_t component)
            {
                return 0;
            }
        }    // namespace digitalOut

        namespace analog
        {
            __attribute__((weak)) bool value(size_t index, uint16_t& value)
            {
                return 0;
            }
        }    // namespace analog

        namespace indicators
        {
            __attribute__((weak)) void indicateTraffic(dataSource_t source, dataDirection_t direction)
            {
            }
        }    // namespace indicators
    }        // namespace IO

    namespace detail
    {
        namespace USB
        {
            __attribute__((weak)) void init()
            {
            }
        }    // namespace USB

        namespace IO
        {
            __attribute__((weak)) void init()
            {
            }

            namespace digitalIn
            {
                __attribute__((weak)) void init()
                {
                }

                __attribute__((weak)) void update()
                {
                }

                __attribute__((weak)) void flush()
                {
                }
            }    // namespace digitalIn

            namespace digitalOut
            {
                __attribute__((weak)) void init()
                {
                }

                __attribute__((weak)) void update()
                {
                }
            }    // namespace digitalOut

            namespace analog
            {
                __attribute__((weak)) void init()
                {
                }
            }    // namespace analog

            namespace indicators
            {
                __attribute__((weak)) void init()
                {
                }

                __attribute__((weak)) void update()
                {
                }

                __attribute__((weak)) void ledFlashStartup()
                {
                }
            }    // namespace indicators

            namespace unused
            {
                __attribute__((weak)) void init()
                {
                }
            }    // namespace unused

            namespace bootloader
            {
                __attribute__((weak)) void init()
                {
                }
            }    // namespace bootloader
        }        // namespace IO

        namespace UART
        {
            __attribute__((weak)) uint8_t dmxChannelValue(size_t channel)
            {
                return 0;
            }
        }    // namespace UART
    }        // namespace detail

    namespace USB
    {
        __attribute__((weak)) void onCDCsetLineEncoding(uint32_t baudRate)
        {
        }
    }    // namespace USB
}    // namespace Board