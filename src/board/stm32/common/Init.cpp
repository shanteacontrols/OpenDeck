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
#include "core/src/general/Interrupt.h"
#include "core/src/general/Timing.h"
#include "board/common/io/Helpers.h"
#include "Pins.h"

namespace Board
{
    void uniqueID(uniqueID_t& uid)
    {
        uint32_t id[3];

        id[0] = HAL_GetUIDw0();
        id[1] = HAL_GetUIDw1();
        id[2] = HAL_GetUIDw2();

        for (int i = 0; i < 3; i++)
        {
            for (int j = 0; j < 4; j++)
                uid.uid[(i * 4) + j] = id[i] >> ((3 - j) * 8) & 0xFF;
        }
    }

    namespace detail
    {
        namespace setup
        {
            void bootloader()
            {
                HAL_Init();

                detail::setup::clocks();
                detail::setup::io();
            }

            void application()
            {
                //Reset of all peripherals, Initializes the Flash interface and the Systick
                HAL_Init();

                detail::setup::clocks();

                //add some delay for clocks to stabilize
                core::timing::waitMs(10);

                detail::setup::io();
                detail::setup::adc();
                detail::setup::timers();

                //add some delay and remove initial readout of digital inputs
                core::timing::waitMs(10);
                detail::io::flushInputReadings();
                detail::setup::usb();
            }

            void cdc()
            {
                //Reset of all peripherals, Initializes the Flash interface and the Systick
                HAL_Init();

                detail::setup::clocks();

                //add some delay for clocks to stabilize
                core::timing::waitMs(10);
                detail::setup::timers();
                detail::setup::usb();
            }

            void io()
            {
#ifdef NUMBER_OF_IN_SR
                CORE_IO_CONFIG({ SR_IN_DATA_PORT, SR_IN_DATA_PIN, core::io::pinMode_t::input, core::io::pullMode_t::none, core::io::gpioSpeed_t::medium, 0x00 });
                CORE_IO_CONFIG({ SR_IN_CLK_PORT, SR_IN_CLK_PIN, core::io::pinMode_t::outputPP, core::io::pullMode_t::none, core::io::gpioSpeed_t::medium, 0x00 });
                CORE_IO_CONFIG({ SR_IN_LATCH_PORT, SR_IN_LATCH_PIN, core::io::pinMode_t::outputPP, core::io::pullMode_t::none, core::io::gpioSpeed_t::medium, 0x00 });

                CORE_IO_SET_LOW(SR_IN_CLK_PORT, SR_IN_CLK_PIN);
                CORE_IO_SET_HIGH(SR_IN_LATCH_PORT, SR_IN_LATCH_PIN);
#else
#ifdef NUMBER_OF_BUTTON_ROWS
                for (int i = 0; i < NUMBER_OF_BUTTON_ROWS; i++)
#else
                for (int i = 0; i < MAX_NUMBER_OF_BUTTONS; i++)
#endif
                {
                    core::io::mcuPin_t pin = detail::map::buttonPin(i);

#ifndef BUTTONS_EXT_PULLUPS
                    CORE_IO_CONFIG({ CORE_IO_MCU_PIN_PORT(pin), CORE_IO_MCU_PIN_INDEX(pin), core::io::pinMode_t::input, core::io::pullMode_t::up, core::io::gpioSpeed_t::medium, 0x00 });
#else
                    CORE_IO_CONFIG({ CORE_IO_MCU_PIN_PORT(pin), CORE_IO_MCU_PIN_INDEX(pin), core::io::pinMode_t::input, core::io::pullMode_t::none, core::io::gpioSpeed_t::medium, 0x00 });
#endif
                }
#endif

#ifdef NUMBER_OF_BUTTON_COLUMNS
                CORE_IO_CONFIG({ DEC_BM_PORT_A0, DEC_BM_PIN_A0, core::io::pinMode_t::outputPP, core::io::pullMode_t::none, core::io::gpioSpeed_t::medium, 0x00 });
                CORE_IO_CONFIG({ DEC_BM_PORT_A1, DEC_BM_PIN_A1, core::io::pinMode_t::outputPP, core::io::pullMode_t::none, core::io::gpioSpeed_t::medium, 0x00 });
                CORE_IO_CONFIG({ DEC_BM_PORT_A2, DEC_BM_PIN_A2, core::io::pinMode_t::outputPP, core::io::pullMode_t::none, core::io::gpioSpeed_t::medium, 0x00 });

                CORE_IO_SET_LOW(DEC_BM_PORT_A0, DEC_BM_PIN_A0);
                CORE_IO_SET_LOW(DEC_BM_PORT_A1, DEC_BM_PIN_A1);
                CORE_IO_SET_LOW(DEC_BM_PORT_A2, DEC_BM_PIN_A2);
#endif

#ifdef NUMBER_OF_OUT_SR
                CORE_IO_CONFIG({ SR_OUT_DATA_PORT, SR_OUT_DATA_PIN, core::io::pinMode_t::outputPP, core::io::pullMode_t::none, core::io::gpioSpeed_t::medium, 0x00 });
                CORE_IO_CONFIG({ SR_OUT_CLK_PORT, SR_OUT_CLK_PIN, core::io::pinMode_t::outputPP, core::io::pullMode_t::none, core::io::gpioSpeed_t::medium, 0x00 });
                CORE_IO_CONFIG({ SR_OUT_LATCH_PORT, SR_OUT_LATCH_PIN, core::io::pinMode_t::outputPP, core::io::pullMode_t::none, core::io::gpioSpeed_t::medium, 0x00 });

#ifdef SR_OUT_OE_PORT
                CORE_IO_CONFIG({ SR_OUT_OE_PORT, SR_OUT_OE_PIN, core::io::pinMode_t::outputPP, core::io::pullMode_t::none, core::io::gpioSpeed_t::medium, 0x00 });
#endif

                //init all outputs on shift register
                CORE_IO_SET_LOW(SR_OUT_LATCH_PORT, SR_OUT_LATCH_PIN);

                for (int i = 0; i < MAX_NUMBER_OF_LEDS; i++)
                {
                    EXT_LED_OFF(SR_OUT_DATA_PORT, SR_OUT_DATA_PIN);
                    CORE_IO_SET_HIGH(SR_OUT_CLK_PORT, SR_OUT_CLK_PIN);
                    detail::io::sr595wait();
                    CORE_IO_SET_LOW(SR_OUT_CLK_PORT, SR_OUT_CLK_PIN);
                }

                CORE_IO_SET_HIGH(SR_OUT_LATCH_PORT, SR_OUT_LATCH_PIN);
#ifdef SR_OUT_OE_PORT
                CORE_IO_SET_LOW(SR_OUT_OE_PORT, SR_OUT_OE_PIN);
#endif
#else
#ifdef NUMBER_OF_LED_ROWS
                CORE_IO_CONFIG({ DEC_LM_PORT_A0, DEC_LM_PIN_A0, core::io::pinMode_t::outputPP, core::io::pullMode_t::none, core::io::gpioSpeed_t::medium, 0x00 });
                CORE_IO_CONFIG({ DEC_LM_PORT_A1, DEC_LM_PIN_A1, core::io::pinMode_t::outputPP, core::io::pullMode_t::none, core::io::gpioSpeed_t::medium, 0x00 });
                CORE_IO_CONFIG({ DEC_LM_PORT_A2, DEC_LM_PIN_A2, core::io::pinMode_t::outputPP, core::io::pullMode_t::none, core::io::gpioSpeed_t::medium, 0x00 });

                CORE_IO_SET_LOW(DEC_LM_PORT_A0, DEC_LM_PIN_A0);
                CORE_IO_SET_LOW(DEC_LM_PORT_A1, DEC_LM_PIN_A1);
                CORE_IO_SET_LOW(DEC_LM_PORT_A2, DEC_LM_PIN_A2);

                for (int i = 0; i < NUMBER_OF_LED_ROWS; i++)
#else
                for (int i = 0; i < MAX_NUMBER_OF_LEDS; i++)
#endif
                {
                    core::io::mcuPin_t pin = detail::map::ledPin(i);

#ifdef NUMBER_OF_LED_ROWS
                    //when rows are used from native outputs, use open-drain configuration
                    CORE_IO_CONFIG({ CORE_IO_MCU_PIN_PORT(pin), CORE_IO_MCU_PIN_INDEX(pin), core::io::pinMode_t::outputOD, core::io::pullMode_t::none, core::io::gpioSpeed_t::medium, 0x00 });
#else
                    CORE_IO_CONFIG({ CORE_IO_MCU_PIN_PORT(pin), CORE_IO_MCU_PIN_INDEX(pin), core::io::pinMode_t::outputPP, core::io::pullMode_t::none, core::io::gpioSpeed_t::medium, 0x00 });
#endif
                    EXT_LED_OFF(CORE_IO_MCU_PIN_PORT(pin), CORE_IO_MCU_PIN_INDEX(pin));
                }
#endif

#if MAX_ADC_CHANNELS > 0
                for (int i = 0; i < MAX_ADC_CHANNELS; i++)
                {
                    core::io::mcuPin_t pin = detail::map::adcPin(i);
                    CORE_IO_CONFIG({ CORE_IO_MCU_PIN_PORT(pin), CORE_IO_MCU_PIN_INDEX(pin), core::io::pinMode_t::analog });
                    CORE_IO_SET_LOW(CORE_IO_MCU_PIN_PORT(pin), CORE_IO_MCU_PIN_INDEX(pin));
                }
#endif

#ifdef NUMBER_OF_MUX
                CORE_IO_CONFIG({ MUX_PORT_S0, MUX_PIN_S0, core::io::pinMode_t::outputPP, core::io::pullMode_t::none, core::io::gpioSpeed_t::medium, 0x00 });
                CORE_IO_CONFIG({ MUX_PORT_S1, MUX_PIN_S1, core::io::pinMode_t::outputPP, core::io::pullMode_t::none, core::io::gpioSpeed_t::medium, 0x00 });
                CORE_IO_CONFIG({ MUX_PORT_S2, MUX_PIN_S2, core::io::pinMode_t::outputPP, core::io::pullMode_t::none, core::io::gpioSpeed_t::medium, 0x00 });
#ifdef MUX_PORT_S3
                CORE_IO_CONFIG({ MUX_PORT_S3, MUX_PIN_S3, core::io::pinMode_t::outputPP, core::io::pullMode_t::none, core::io::gpioSpeed_t::medium, 0x00 });
#endif
#endif

#ifdef BTLDR_BUTTON_PORT
#ifdef BTLDR_BUTTON_AH
                CORE_IO_CONFIG({ BTLDR_BUTTON_PORT, BTLDR_BUTTON_PIN, core::io::pinMode_t::input, core::io::pullMode_t::down, core::io::gpioSpeed_t::medium, 0x00 });
#else
                CORE_IO_CONFIG({ BTLDR_BUTTON_PORT, BTLDR_BUTTON_PIN, core::io::pinMode_t::input, core::io::pullMode_t::up, core::io::gpioSpeed_t::medium, 0x00 });
#endif
#endif

#ifdef LED_INDICATORS
                CORE_IO_CONFIG({ LED_MIDI_IN_DIN_PORT, LED_MIDI_IN_DIN_PIN, core::io::pinMode_t::outputPP, core::io::pullMode_t::none, core::io::gpioSpeed_t::medium, 0x00 });
                CORE_IO_CONFIG({ LED_MIDI_OUT_DIN_PORT, LED_MIDI_OUT_DIN_PIN, core::io::pinMode_t::outputPP, core::io::pullMode_t::none, core::io::gpioSpeed_t::medium, 0x00 });
                CORE_IO_CONFIG({ LED_MIDI_IN_USB_PORT, LED_MIDI_IN_USB_PIN, core::io::pinMode_t::outputPP, core::io::pullMode_t::none, core::io::gpioSpeed_t::medium, 0x00 });
                CORE_IO_CONFIG({ LED_MIDI_OUT_USB_PORT, LED_MIDI_OUT_USB_PIN, core::io::pinMode_t::outputPP, core::io::pullMode_t::none, core::io::gpioSpeed_t::medium, 0x00 });

                INT_LED_OFF(LED_MIDI_IN_DIN_PORT, LED_MIDI_IN_DIN_PIN);
                INT_LED_OFF(LED_MIDI_OUT_DIN_PORT, LED_MIDI_OUT_DIN_PIN);
                INT_LED_OFF(LED_MIDI_IN_USB_PORT, LED_MIDI_IN_USB_PIN);
                INT_LED_OFF(LED_MIDI_OUT_USB_PORT, LED_MIDI_OUT_USB_PIN);
#endif

#ifdef LED_BTLDR_PORT
                CORE_IO_CONFIG({ LED_BTLDR_PORT, LED_BTLDR_PIN, core::io::pinMode_t::outputPP, core::io::pullMode_t::none, core::io::gpioSpeed_t::medium, 0x00 });
                CORE_IO_SET_HIGH(LED_BTLDR_PORT, LED_BTLDR_PIN);
#endif

#ifdef TOTAL_UNUSED_IO
                for (int i = 0; i < TOTAL_UNUSED_IO; i++)
                {
                    core::io::mcuPin_t pin = detail::map::unusedPin(i);
                    CORE_IO_CONFIG({ CORE_IO_MCU_PIN_PORT(pin), CORE_IO_MCU_PIN_INDEX(pin), core::io::pinMode_t::outputPP });
                    CORE_IO_SET_STATE(CORE_IO_MCU_PIN_PORT(pin), CORE_IO_MCU_PIN_INDEX(pin), detail::map::unusedPinState(i));
                }
#endif
            }
        }    // namespace setup
    }        // namespace detail
}    // namespace Board