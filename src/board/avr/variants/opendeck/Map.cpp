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

#include "Pins.h"
#include "board/common/Map.h"

///
/// brief Pin and index remapping used to match indexes on PCB silkscreen.
/// @{

#if HARDWARE_VERSION_MINOR == 1
#define MUX_Y0 8
#define MUX_Y1 9
#define MUX_Y2 10
#define MUX_Y3 11
#define MUX_Y4 12
#define MUX_Y5 13
#define MUX_Y6 14
#define MUX_Y7 15
#define MUX_Y8 7
#define MUX_Y9 6
#define MUX_Y10 5
#define MUX_Y11 4
#define MUX_Y12 3
#define MUX_Y13 2
#define MUX_Y14 1
#define MUX_Y15 0
#elif HARDWARE_VERSION_MINOR == 2
#define MUX_Y0 8
#define MUX_Y1 9
#define MUX_Y2 10
#define MUX_Y3 11
#define MUX_Y4 12
#define MUX_Y5 13
#define MUX_Y6 14
#define MUX_Y7 15
#define MUX_Y8 4
#define MUX_Y9 5
#define MUX_Y10 7
#define MUX_Y11 6
#define MUX_Y12 0
#define MUX_Y13 1
#define MUX_Y14 2
#define MUX_Y15 3
#endif

#define DM_ROW_1_BIT 0
#define DM_ROW_2_BIT 1
#define DM_ROW_3_BIT 2
#define DM_ROW_4_BIT 3
#define DM_ROW_5_BIT 7
#define DM_ROW_6_BIT 6
#define DM_ROW_7_BIT 5
#define DM_ROW_8_BIT 4

#define DM_COLUMN_1 0
#define DM_COLUMN_2 7
#define DM_COLUMN_3 2
#define DM_COLUMN_4 1
#define DM_COLUMN_5 4
#define DM_COLUMN_6 3
#define DM_COLUMN_7 5
#define DM_COLUMN_8 6

namespace Board
{
    namespace map
    {
        namespace
        {
            const uint8_t muxPinOrderArray[NUMBER_OF_MUX_INPUTS] = {
                MUX_Y0,
                MUX_Y1,
                MUX_Y2,
                MUX_Y3,
                MUX_Y4,
                MUX_Y5,
                MUX_Y6,
                MUX_Y7,
                MUX_Y8,
                MUX_Y9,
                MUX_Y10,
                MUX_Y11,
                MUX_Y12,
                MUX_Y13,
                MUX_Y14,
                MUX_Y15
            };

            //row bits are stored in inverse order when performing read
            const uint8_t dmRowBitArray[NUMBER_OF_BUTTON_ROWS] = {
                DM_ROW_8_BIT,
                DM_ROW_7_BIT,
                DM_ROW_6_BIT,
                DM_ROW_5_BIT,
                DM_ROW_4_BIT,
                DM_ROW_3_BIT,
                DM_ROW_2_BIT,
                DM_ROW_1_BIT
            };

            const uint8_t dmColumnArray[NUMBER_OF_BUTTON_COLUMNS] = {
                DM_COLUMN_1,
                DM_COLUMN_2,
                DM_COLUMN_3,
                DM_COLUMN_4,
                DM_COLUMN_5,
                DM_COLUMN_6,
                DM_COLUMN_7,
                DM_COLUMN_8
            };

            ///
            /// \brief Array holding ADC read pins/channels.
            ///
            const core::io::mcuPin_t aInPins[NUMBER_OF_MUX] = {
                //port not used on avr mcus for adc channels
                //adc channel doesn't have to match with adc pin
                {
                    .port = nullptr,
                    .index = MUX_1_IN_PIN,
                },

                {
                    .port = nullptr,
                    .index = MUX_2_IN_PIN,
                }
            };

            ///
            /// \brief Array of mcuPin_t structure holding port/pin for every LED row for easier access.
            ///
            const core::io::mcuPin_t ledRowPins[NUMBER_OF_LED_ROWS] = {
                {
                    .port = &LED_ROW_1_PORT,
                    .index = LED_ROW_1_PIN,
                },

                {
                    .port = &LED_ROW_2_PORT,
                    .index = LED_ROW_2_PIN,
                },

                {
                    .port = &LED_ROW_3_PORT,
                    .index = LED_ROW_3_PIN,
                },

                {
                    .port = &LED_ROW_4_PORT,
                    .index = LED_ROW_4_PIN,
                },

                {
                    .port = &LED_ROW_5_PORT,
                    .index = LED_ROW_5_PIN,
                },

                {
                    .port = &LED_ROW_6_PORT,
                    .index = LED_ROW_6_PIN,
                }
            };

            const core::io::pwmChannel_t pwmChannels[NUMBER_OF_LED_ROWS] = {
                { .timer = &TCCR1A,
                  .compareL = &OCR1CL,
                  .compareH = &OCR1CH,
                  .channel = COM1C1 },

                { .timer = &TCCR4C,
                  .compareL = &OCR4D,
                  .compareH = nullptr,
                  .channel = COM4D1 },

                { .timer = &TCCR1A,
                  .compareL = &OCR1AL,
                  .compareH = &OCR1AH,
                  .channel = COM1A1 },

                { .timer = &TCCR4A,
                  .compareL = &OCR4A,
                  .compareH = nullptr,
                  .channel = COM3A1 },

                { .timer = &TCCR3A,
                  .compareL = &OCR3AL,
                  .compareH = &OCR3AH,
                  .channel = COM1C1 },

                { .timer = &TCCR1A,
                  .compareL = &OCR1BL,
                  .compareH = &OCR1BH,
                  .channel = COM1B1 }
            };
        }    // namespace

        core::io::mcuPin_t adcChannel(uint8_t index)
        {
            return aInPins[index];
        }

        uint8_t muxChannel(uint8_t index)
        {
            return muxPinOrderArray[index];
        }

        uint8_t inMatrixRow(uint8_t index)
        {
            return dmRowBitArray[index];
        }

        uint8_t inMatrixColumn(uint8_t index)
        {
            return dmColumnArray[index];
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
}    // namespace Board

/// @}
