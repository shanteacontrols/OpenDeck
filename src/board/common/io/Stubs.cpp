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

#include "board/Board.h"
#include "board/Internal.h"

// stub functions used so that firmware can be compiled without resorting to ifdef mess if
// some IO module isn't supported

namespace Board
{
    namespace io
    {
        __attribute__((weak)) bool digitalInState(size_t digitalInIndex, dInReadings_t& dInReadings)
        {
            return false;
        }

        __attribute__((weak)) size_t encoderIndex(size_t buttonID)
        {
            return 0;
        }

        __attribute__((weak)) size_t encoderSignalIndex(size_t encoderID, encoderIndex_t index)
        {
            return 0;
        }

        __attribute__((weak)) void writeLEDstate(size_t ledID, bool state)
        {
        }

        __attribute__((weak)) size_t rgbIndex(size_t ledID)
        {
            return 0;
        }

        __attribute__((weak)) size_t rgbSignalIndex(size_t rgbID, rgbIndex_t index)
        {
            return 0;
        }

        __attribute__((weak)) uint16_t getAnalogValue(size_t analogID)
        {
            return 0;
        }

        __attribute__((weak)) void indicateTraffic(Board::io::dataSource_t source, Board::io::dataDirection_t direction)
        {
        }
    }    // namespace io

    namespace detail
    {
        namespace io
        {
            __attribute__((weak)) void checkDigitalInputs()
            {
            }

            __attribute__((weak)) void flushInputReadings()
            {
            }

            __attribute__((weak)) void checkDigitalOutputs()
            {
            }

            __attribute__((weak)) void checkIndicators()
            {
            }

            __attribute__((weak)) void ledFlashStartup()
            {
            }
        }    // namespace io

        namespace setup
        {
            __attribute__((weak)) void usb()
            {
            }

            __attribute__((weak)) void adc()
            {
            }
        }    // namespace setup
    }        // namespace detail

    namespace USB
    {
        __attribute__((weak)) void onCDCsetLineEncoding(uint32_t baudRate)
        {
        }
    }    // namespace USB
}    // namespace Board