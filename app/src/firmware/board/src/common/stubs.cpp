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

#include "board/board.h"
#include "internal.h"

// Stub functions are used so that firmware can be compiled without
// resorting to ifdef mess if something isn't supported

namespace board
{
    __attribute__((weak)) void update()
    {
    }

    namespace io
    {
        namespace digital_in
        {
            __attribute__((weak)) bool state(size_t index, Readings& readings)
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
        }    // namespace digital_in

        namespace digital_out
        {
            __attribute__((weak)) void writeLedState(size_t index, ledBrightness_t ledBrightness)
            {
            }

            __attribute__((weak)) size_t rgbFromOutput(size_t index)
            {
                return 0;
            }

            __attribute__((weak)) size_t rgbComponentFromRgb(size_t index, rgbComponent_t component)
            {
                return 0;
            }
        }    // namespace digital_out

        namespace analog
        {
            __attribute__((weak)) bool value(size_t index, uint16_t& value)
            {
                return 0;
            }
        }    // namespace analog

        namespace indicators
        {
            __attribute__((weak)) void indicateTraffic(source_t source, direction_t direction)
            {
            }

            __attribute__((weak)) void indicateFirmwareUpdateStart()
            {
            }

            __attribute__((weak)) void indicateFactoryReset()
            {
            }
        }    // namespace indicators
    }    // namespace io

    namespace detail::io
    {
        __attribute__((weak)) void init()
        {
        }

        namespace digital_in
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
        }    // namespace digital_in

        namespace digital_out
        {
            __attribute__((weak)) void init()
            {
            }

            __attribute__((weak)) void update()
            {
            }
        }    // namespace digital_out

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

            __attribute__((weak)) void indicateApplicationLoad()
            {
            }

            __attribute__((weak)) void indicateBootloaderLoad()
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
    }    // namespace detail::io
}    // namespace board