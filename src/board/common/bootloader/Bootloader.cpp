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
#include "board/common/io/Helpers.h"
#include "Pins.h"
#include "core/src/general/Timing.h"
#include "common/OpenDeckMIDIformat/OpenDeckMIDIformat.h"

namespace Board
{
    namespace detail
    {
        namespace bootloader
        {
            btldrTrigger_t btldrTrigger()
            {
                //add some delay before reading the pins to avoid incorrect state detection
                core::timing::waitMs(100);

                bool hardwareTrigger = Board::detail::bootloader::isHWtriggerActive();

                //check if user wants to enter bootloader
                bool softwareTrigger = Board::detail::bootloader::isSWtriggerActive();

                detail::bootloader::clearSWtrigger();

                if (softwareTrigger && hardwareTrigger)
                    return btldrTrigger_t::all;
                else if (!softwareTrigger && hardwareTrigger)
                    return btldrTrigger_t::hardware;
                else if (softwareTrigger && !hardwareTrigger)
                    return btldrTrigger_t::software;
                else
                    return btldrTrigger_t::none;
            }

            void indicate()
            {
#if defined(LED_INDICATORS)
                INT_LED_ON(LED_MIDI_IN_DIN_PORT, LED_MIDI_IN_DIN_PIN);
                INT_LED_ON(LED_MIDI_OUT_DIN_PORT, LED_MIDI_OUT_DIN_PIN);
                INT_LED_ON(LED_MIDI_IN_USB_PORT, LED_MIDI_IN_USB_PIN);
                INT_LED_ON(LED_MIDI_OUT_USB_PORT, LED_MIDI_OUT_USB_PIN);
#elif defined(NUMBER_OF_OUT_SR) && !defined(NUMBER_OF_LED_COLUMNS)
                //turn on all available LEDs
                CORE_IO_SET_LOW(SR_OUT_LATCH_PORT, SR_OUT_LATCH_PIN);

                for (int i = 0; i < NUMBER_OF_OUT_SR; i++)
                {
                    for (int j = 0; j < 8; j++)
                    {
                        EXT_LED_ON(SR_OUT_DATA_PORT, SR_OUT_DATA_PIN);
                        CORE_IO_SET_LOW(SR_OUT_CLK_PORT, SR_OUT_CLK_PIN);
                        _NOP();
                        _NOP();
                        CORE_IO_SET_HIGH(SR_OUT_CLK_PORT, SR_OUT_CLK_PIN);
                    }
                }

                CORE_IO_SET_HIGH(SR_OUT_LATCH_PORT, SR_OUT_LATCH_PIN);
#elif defined(LED_BTLDR_PORT)
                //only one led
                INT_LED_ON(LED_BTLDR_PORT, LED_BTLDR_PIN);
#endif
            }

            bool isHWtriggerActive()
            {
#if defined(BTLDR_BUTTON_INDEX)
                CORE_IO_SET_LOW(SR_IN_CLK_PORT, SR_IN_CLK_PIN);
                CORE_IO_SET_LOW(SR_IN_LATCH_PORT, SR_IN_LATCH_PIN);
                _NOP();

                CORE_IO_SET_HIGH(SR_IN_LATCH_PORT, SR_IN_LATCH_PIN);

                for (int j = 0; j < NUMBER_OF_IN_SR; j++)
                {
                    for (int i = 0; i < 8; i++)
                    {
                        uint8_t index = (7 - i) + j * 8;

                        if (index == BTLDR_BUTTON_INDEX)
                        {
                            if (!CORE_IO_READ(SR_IN_DATA_PORT, SR_IN_DATA_PIN))
                                return true;
                        }

                        CORE_IO_SET_LOW(SR_IN_CLK_PORT, SR_IN_CLK_PIN);
                        _NOP();
                        CORE_IO_SET_HIGH(SR_IN_CLK_PORT, SR_IN_CLK_PIN);
                    }
                }

                return false;
#elif defined(BTLDR_BUTTON_PORT)
                return !CORE_IO_READ(BTLDR_BUTTON_PORT, BTLDR_BUTTON_PIN);
#else
                //no hardware entry possible in this case
                return false;
#endif
            }
        }    // namespace bootloader
    }        // namespace detail
}    // namespace Board