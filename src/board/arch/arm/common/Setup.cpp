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
#include "board/common/io/Helpers.h"
#include "core/src/general/Timing.h"
#include <Target.h>

namespace Board::detail::setup
{
    void bootloader()
    {
        detail::setup::halInit();
        detail::setup::clocks();
        detail::setup::io();
    }

    void application()
    {
        detail::setup::halInit();
        detail::setup::clocks();

        // add some delay for clocks to stabilize
        core::timing::waitMs(10);

        detail::setup::io();
        detail::setup::adc();
        detail::setup::timers();

        // add some delay and remove initial readout of digital inputs
        core::timing::waitMs(10);
        detail::io::flushInputReadings();
        detail::setup::usb();
    }

    void io()
    {
#ifdef DIGITAL_INPUTS_SUPPORTED
#ifdef NUMBER_OF_IN_SR
        CORE_IO_INIT({ SR_IN_DATA_PORT, SR_IN_DATA_PIN, core::io::pinMode_t::INPUT, core::io::pullMode_t::NONE });
        CORE_IO_INIT({ SR_IN_CLK_PORT, SR_IN_CLK_PIN, core::io::pinMode_t::OUTPUT_PP, core::io::pullMode_t::NONE });
        CORE_IO_INIT({ SR_IN_LATCH_PORT, SR_IN_LATCH_PIN, core::io::pinMode_t::OUTPUT_PP, core::io::pullMode_t::NONE });

        CORE_IO_SET_LOW(SR_IN_CLK_PORT, SR_IN_CLK_PIN);
        CORE_IO_SET_HIGH(SR_IN_LATCH_PORT, SR_IN_LATCH_PIN);
#else
#ifdef NUMBER_OF_BUTTON_ROWS
        for (int i = 0; i < NUMBER_OF_BUTTON_ROWS; i++)
#else
        for (int i = 0; i < NR_OF_DIGITAL_INPUTS; i++)
#endif
        {
            core::io::mcuPin_t pin = detail::map::buttonPin(i);

#ifndef BUTTONS_EXT_PULLUPS
            CORE_IO_INIT({ CORE_IO_MCU_PIN_PORT(pin), CORE_IO_MCU_PIN_INDEX(pin), core::io::pinMode_t::INPUT, core::io::pullMode_t::UP });
#else
            CORE_IO_INIT({ CORE_IO_MCU_PIN_PORT(pin), CORE_IO_MCU_PIN_INDEX(pin), core::io::pinMode_t::INPUT, core::io::pullMode_t::NONE });
#endif
        }
#endif

#ifdef NUMBER_OF_BUTTON_COLUMNS
        CORE_IO_INIT({ DEC_BM_PORT_A0, DEC_BM_PIN_A0, core::io::pinMode_t::OUTPUT_PP, core::io::pullMode_t::NONE });
        CORE_IO_INIT({ DEC_BM_PORT_A1, DEC_BM_PIN_A1, core::io::pinMode_t::OUTPUT_PP, core::io::pullMode_t::NONE });
        CORE_IO_INIT({ DEC_BM_PORT_A2, DEC_BM_PIN_A2, core::io::pinMode_t::OUTPUT_PP, core::io::pullMode_t::NONE });

        CORE_IO_SET_LOW(DEC_BM_PORT_A0, DEC_BM_PIN_A0);
        CORE_IO_SET_LOW(DEC_BM_PORT_A1, DEC_BM_PIN_A1);
        CORE_IO_SET_LOW(DEC_BM_PORT_A2, DEC_BM_PIN_A2);
#endif
#endif

#ifdef DIGITAL_OUTPUTS_SUPPORTED
#ifdef NUMBER_OF_OUT_SR
        CORE_IO_INIT({ SR_OUT_DATA_PORT, SR_OUT_DATA_PIN, core::io::pinMode_t::OUTPUT_PP, core::io::pullMode_t::NONE });
        CORE_IO_INIT({ SR_OUT_CLK_PORT, SR_OUT_CLK_PIN, core::io::pinMode_t::OUTPUT_PP, core::io::pullMode_t::NONE });
        CORE_IO_INIT({ SR_OUT_LATCH_PORT, SR_OUT_LATCH_PIN, core::io::pinMode_t::OUTPUT_PP, core::io::pullMode_t::NONE });

#ifdef SR_OUT_OE_PORT
        CORE_IO_INIT({ SR_OUT_OE_PORT, SR_OUT_OE_PIN, core::io::pinMode_t::OUTPUT_PP, core::io::pullMode_t::NONE });
#endif

        // init all outputs on shift register
        CORE_IO_SET_LOW(SR_OUT_LATCH_PORT, SR_OUT_LATCH_PIN);

        for (int i = 0; i < NR_OF_DIGITAL_OUTPUTS; i++)
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
        CORE_IO_INIT({ DEC_LM_PORT_A0, DEC_LM_PIN_A0, core::io::pinMode_t::OUTPUT_PP, core::io::pullMode_t::NONE });
        CORE_IO_INIT({ DEC_LM_PORT_A1, DEC_LM_PIN_A1, core::io::pinMode_t::OUTPUT_PP, core::io::pullMode_t::NONE });
        CORE_IO_INIT({ DEC_LM_PORT_A2, DEC_LM_PIN_A2, core::io::pinMode_t::OUTPUT_PP, core::io::pullMode_t::NONE });

        CORE_IO_SET_LOW(DEC_LM_PORT_A0, DEC_LM_PIN_A0);
        CORE_IO_SET_LOW(DEC_LM_PORT_A1, DEC_LM_PIN_A1);
        CORE_IO_SET_LOW(DEC_LM_PORT_A2, DEC_LM_PIN_A2);

        for (int i = 0; i < NUMBER_OF_LED_ROWS; i++)
#else
        for (int i = 0; i < NR_OF_DIGITAL_OUTPUTS; i++)
#endif
        {
            core::io::mcuPin_t pin = detail::map::ledPin(i);

#ifdef NUMBER_OF_LED_ROWS
            // when rows are used from native outputs, use open-drain configuration
            CORE_IO_INIT({ CORE_IO_MCU_PIN_PORT(pin), CORE_IO_MCU_PIN_INDEX(pin), core::io::pinMode_t::OUTPUT_OD, core::io::pullMode_t::NONE });
#else
            CORE_IO_INIT({ CORE_IO_MCU_PIN_PORT(pin), CORE_IO_MCU_PIN_INDEX(pin), core::io::pinMode_t::OUTPUT_PP, core::io::pullMode_t::NONE });
#endif
            EXT_LED_OFF(CORE_IO_MCU_PIN_PORT(pin), CORE_IO_MCU_PIN_INDEX(pin));
        }
#endif
#endif

#if MAX_ADC_CHANNELS > 0
        for (int i = 0; i < MAX_ADC_CHANNELS; i++)
        {
            core::io::mcuPin_t pin = detail::map::adcPin(i);
            CORE_IO_INIT({ CORE_IO_MCU_PIN_PORT(pin), CORE_IO_MCU_PIN_INDEX(pin), core::io::pinMode_t::ANALOG });
            CORE_IO_SET_LOW(CORE_IO_MCU_PIN_PORT(pin), CORE_IO_MCU_PIN_INDEX(pin));
        }
#endif

#ifdef NUMBER_OF_MUX
        CORE_IO_INIT({ MUX_PORT_S0, MUX_PIN_S0, core::io::pinMode_t::OUTPUT_PP, core::io::pullMode_t::NONE });
        CORE_IO_INIT({ MUX_PORT_S1, MUX_PIN_S1, core::io::pinMode_t::OUTPUT_PP, core::io::pullMode_t::NONE });
        CORE_IO_INIT({ MUX_PORT_S2, MUX_PIN_S2, core::io::pinMode_t::OUTPUT_PP, core::io::pullMode_t::NONE });
#ifdef MUX_PORT_S3
        CORE_IO_INIT({ MUX_PORT_S3, MUX_PIN_S3, core::io::pinMode_t::OUTPUT_PP, core::io::pullMode_t::NONE });
#endif
#endif

#ifdef BTLDR_BUTTON_PORT
#ifdef BTLDR_BUTTON_AH
        CORE_IO_INIT({ BTLDR_BUTTON_PORT, BTLDR_BUTTON_PIN, core::io::pinMode_t::INPUT, core::io::pullMode_t::DOWN });
#else
        CORE_IO_INIT({ BTLDR_BUTTON_PORT, BTLDR_BUTTON_PIN, core::io::pinMode_t::INPUT, core::io::pullMode_t::UP });
#endif
#endif

#ifdef TOTAL_UNUSED_IO
        for (int i = 0; i < TOTAL_UNUSED_IO; i++)
        {
            Board::detail::io::unusedIO_t unusedPin = detail::map::unusedPin(i);

            CORE_IO_INIT({ unusedPin.pin.port, unusedPin.pin.index, unusedPin.pin.mode });

            // for input mode, pull up is activated so no need to set state via CORE_IO_SET_STATE
            if (unusedPin.pin.mode == core::io::pinMode_t::OUTPUT_PP)
                CORE_IO_SET_STATE(CORE_IO_MCU_PIN_PORT(unusedPin.pin), CORE_IO_MCU_PIN_INDEX(unusedPin.pin), unusedPin.state);
        }
#endif

#ifdef LED_INDICATORS
        CORE_IO_INIT({ LED_MIDI_IN_DIN_PORT, LED_MIDI_IN_DIN_PIN, core::io::pinMode_t::OUTPUT_PP, core::io::pullMode_t::NONE });
        CORE_IO_INIT({ LED_MIDI_OUT_DIN_PORT, LED_MIDI_OUT_DIN_PIN, core::io::pinMode_t::OUTPUT_PP, core::io::pullMode_t::NONE });
        CORE_IO_INIT({ LED_MIDI_IN_USB_PORT, LED_MIDI_IN_USB_PIN, core::io::pinMode_t::OUTPUT_PP, core::io::pullMode_t::NONE });
        CORE_IO_INIT({ LED_MIDI_OUT_USB_PORT, LED_MIDI_OUT_USB_PIN, core::io::pinMode_t::OUTPUT_PP, core::io::pullMode_t::NONE });

        INT_LED_OFF(LED_MIDI_IN_DIN_PORT, LED_MIDI_IN_DIN_PIN);
        INT_LED_OFF(LED_MIDI_OUT_DIN_PORT, LED_MIDI_OUT_DIN_PIN);
        INT_LED_OFF(LED_MIDI_IN_USB_PORT, LED_MIDI_IN_USB_PIN);
        INT_LED_OFF(LED_MIDI_OUT_USB_PORT, LED_MIDI_OUT_USB_PIN);
#endif
    }
}    // namespace Board::detail::setup