/*

Copyright 2015-2020 Igor Petrovic

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

//stub functions used so that firmware can be compiled without resorting to ifdef mess if
//some IO module isn't supported

namespace Board
{
    namespace io
    {
        __attribute__((weak)) bool isInputDataAvailable()
        {
            return false;
        }

        __attribute__((weak)) bool getButtonState(uint8_t buttonIndex)
        {
            return false;
        }

        __attribute__((weak)) uint8_t getEncoderPair(uint8_t buttonID)
        {
            return 0;
        }

        __attribute__((weak)) uint8_t getEncoderPairState(uint8_t encoderID)
        {
            return 0;
        }

        __attribute__((weak)) void writeLEDstate(uint8_t ledID, bool state)
        {
        }

        __attribute__((weak)) uint8_t getRGBaddress(uint8_t rgbID, rgbIndex_t index)
        {
            return 0;
        }

        __attribute__((weak)) uint8_t getRGBID(uint8_t ledID)
        {
            return 0;
        }

        __attribute__((weak)) void setLEDfadeSpeed(uint8_t transitionSpeed)
        {
        }

        __attribute__((weak)) bool isAnalogDataAvailable()
        {
            return false;
        }

        __attribute__((weak)) uint16_t getAnalogValue(uint8_t analogID)
        {
            return 0;
        }
    }    // namespace io

    namespace detail
    {
        namespace io
        {
            __attribute__((weak)) void checkDigitalInputs()
            {
            }

            __attribute__((weak)) void checkDigitalOutputs()
            {
            }

            __attribute__((weak)) void indicateMIDItraffic(MIDI::interface_t source, midiTrafficDirection_t direction)
            {
            }

            __attribute__((weak)) void checkIndicators()
            {
            }

            __attribute__((weak)) void enableIndicators()
            {
            }
            __attribute__((weak)) void disableIndicators()
            {
            }

            __attribute__((weak)) void ledFlashStartup(bool fwUpdated)
            {
            }
        }    // namespace io
    }        // namespace detail
}    // namespace Board