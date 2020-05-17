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

#include "Pins.h"
#include "board/Internal.h"

namespace Board
{
    namespace detail
    {
        namespace map
        {
            namespace
            {
                uint8_t aInChannels[NUMBER_OF_MUX] = {
                    MUX_1_IN_PIN,
                    MUX_2_IN_PIN,
                    MUX_3_IN_PIN,
                    MUX_4_IN_PIN,
                    MUX_5_IN_PIN,
                    MUX_6_IN_PIN,
                    MUX_7_IN_PIN
                };

                ///
                /// \brief Array of mcuPin_t structure holding port/pin for every LED row for easier access.
                ///
                const core::io::mcuPin_t ledRowPins[NUMBER_OF_LED_ROWS] = {
                    CORE_IO_MCU_PIN_DEF(LED_ROW_1_PORT, LED_ROW_1_PIN),
                    CORE_IO_MCU_PIN_DEF(LED_ROW_2_PORT, LED_ROW_2_PIN),
                    CORE_IO_MCU_PIN_DEF(LED_ROW_3_PORT, LED_ROW_3_PIN),
                    CORE_IO_MCU_PIN_DEF(LED_ROW_4_PORT, LED_ROW_4_PIN),
                    CORE_IO_MCU_PIN_DEF(LED_ROW_5_PORT, LED_ROW_5_PIN),
                    CORE_IO_MCU_PIN_DEF(LED_ROW_6_PORT, LED_ROW_6_PIN)
                };

                const core::io::pwmChannel_t pwmChannels[NUMBER_OF_LED_ROWS] = {
                    {
                        .controlRegister = &TCCR4A,
                        .compareValueL   = &OCR4AL,
                        .compareValueH   = &OCR4AH,
                        .compareOutMode  = (1 << COM4A1),
                    },

                    {
                        .controlRegister = &TCCR4A,
                        .compareValueL   = &OCR4BL,
                        .compareValueH   = &OCR4BH,
                        .compareOutMode  = (1 << COM4B1),
                    },

                    {
                        .controlRegister = &TCCR4A,
                        .compareValueL   = &OCR4CL,
                        .compareValueH   = &OCR4CH,
                        .compareOutMode  = (1 << COM4C1),
                    },

                    {
                        .controlRegister = &TCCR3A,
                        .compareValueL   = &OCR3AL,
                        .compareValueH   = &OCR3AH,
                        .compareOutMode  = (1 << COM3A1),
                    },

                    {
                        .controlRegister = &TCCR3A,
                        .compareValueL   = &OCR3BL,
                        .compareValueH   = &OCR3BH,
                        .compareOutMode  = (1 << COM3B1),
                    },

                    {
                        .controlRegister = &TCCR3A,
                        .compareValueL   = &OCR3CL,
                        .compareValueH   = &OCR3CH,
                        .compareOutMode  = (1 << COM3C1),
                    }
                };
            }    // namespace

            uint32_t adcChannel(uint8_t index)
            {
                return aInChannels[index];
            }

            uint8_t muxChannel(uint8_t index)
            {
                return index;
            }

            uint8_t inMatrixRow(uint8_t index)
            {
                return index;
            }

            uint8_t inMatrixColumn(uint8_t index)
            {
                return index;
            }

            core::io::mcuPin_t led(uint8_t index)
            {
                return ledRowPins[index];
            }

            core::io::pwmChannel_t pwmChannel(uint8_t index)
            {
                return pwmChannels[index];
            }
        }    // namespace map
    }        // namespace detail
}    // namespace Board