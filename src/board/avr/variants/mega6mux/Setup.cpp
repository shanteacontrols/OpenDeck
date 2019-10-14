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

#include <util/delay.h>
#include "board/Board.h"
#include "Pins.h"
#include "board/common/Map.h"
#include "board/common/constants/LEDs.h"
#include "core/src/avr/PinManipulation.h"
#include "core/src/avr/ADC.h"
#include "common/OpenDeckMIDIformat/OpenDeckMIDIformat.h"

namespace Board
{
    namespace setup
    {
        void pins()
        {
            CORE_AVR_PIN_SET_INPUT(DI_1_PORT, DI_1_PIN);
            CORE_AVR_PIN_SET_HIGH(DI_1_PORT, DI_1_PIN);

            CORE_AVR_PIN_SET_INPUT(DI_2_PORT, DI_2_PIN);
            CORE_AVR_PIN_SET_HIGH(DI_2_PORT, DI_2_PIN);

            CORE_AVR_PIN_SET_INPUT(DI_3_PORT, DI_3_PIN);
            CORE_AVR_PIN_SET_HIGH(DI_3_PORT, DI_3_PIN);

            CORE_AVR_PIN_SET_INPUT(DI_4_PORT, DI_4_PIN);
            CORE_AVR_PIN_SET_HIGH(DI_4_PORT, DI_4_PIN);

            CORE_AVR_PIN_SET_INPUT(DI_5_PORT, DI_5_PIN);
            CORE_AVR_PIN_SET_HIGH(DI_5_PORT, DI_5_PIN);

            CORE_AVR_PIN_SET_INPUT(DI_6_PORT, DI_6_PIN);
            CORE_AVR_PIN_SET_HIGH(DI_6_PORT, DI_6_PIN);

            CORE_AVR_PIN_SET_INPUT(DI_7_PORT, DI_7_PIN);
            CORE_AVR_PIN_SET_HIGH(DI_7_PORT, DI_7_PIN);

            CORE_AVR_PIN_SET_INPUT(DI_8_PORT, DI_8_PIN);
            CORE_AVR_PIN_SET_HIGH(DI_8_PORT, DI_8_PIN);

            CORE_AVR_PIN_SET_INPUT(DI_9_PORT, DI_9_PIN);
            CORE_AVR_PIN_SET_HIGH(DI_9_PORT, DI_9_PIN);

            CORE_AVR_PIN_SET_INPUT(DI_10_PORT, DI_10_PIN);
            CORE_AVR_PIN_SET_HIGH(DI_10_PORT, DI_10_PIN);

            CORE_AVR_PIN_SET_INPUT(DI_11_PORT, DI_11_PIN);
            CORE_AVR_PIN_SET_HIGH(DI_11_PORT, DI_11_PIN);

            CORE_AVR_PIN_SET_INPUT(DI_12_PORT, DI_12_PIN);
            CORE_AVR_PIN_SET_HIGH(DI_12_PORT, DI_12_PIN);

            CORE_AVR_PIN_SET_INPUT(DI_13_PORT, DI_13_PIN);
            CORE_AVR_PIN_SET_HIGH(DI_13_PORT, DI_13_PIN);

            CORE_AVR_PIN_SET_INPUT(DI_14_PORT, DI_14_PIN);
            CORE_AVR_PIN_SET_HIGH(DI_14_PORT, DI_14_PIN);

            CORE_AVR_PIN_SET_INPUT(DI_15_PORT, DI_15_PIN);
            CORE_AVR_PIN_SET_HIGH(DI_15_PORT, DI_15_PIN);

            CORE_AVR_PIN_SET_INPUT(DI_16_PORT, DI_16_PIN);
            CORE_AVR_PIN_SET_HIGH(DI_16_PORT, DI_16_PIN);

            CORE_AVR_PIN_SET_INPUT(DI_17_PORT, DI_17_PIN);
            CORE_AVR_PIN_SET_HIGH(DI_17_PORT, DI_17_PIN);

            CORE_AVR_PIN_SET_INPUT(DI_18_PORT, DI_18_PIN);
            CORE_AVR_PIN_SET_HIGH(DI_18_PORT, DI_18_PIN);

            CORE_AVR_PIN_SET_INPUT(DI_19_PORT, DI_19_PIN);
            CORE_AVR_PIN_SET_HIGH(DI_19_PORT, DI_19_PIN);

            CORE_AVR_PIN_SET_INPUT(DI_20_PORT, DI_20_PIN);
            CORE_AVR_PIN_SET_HIGH(DI_20_PORT, DI_20_PIN);

            CORE_AVR_PIN_SET_INPUT(DI_21_PORT, DI_21_PIN);
            CORE_AVR_PIN_SET_HIGH(DI_21_PORT, DI_21_PIN);

            CORE_AVR_PIN_SET_INPUT(DI_22_PORT, DI_22_PIN);
            CORE_AVR_PIN_SET_HIGH(DI_22_PORT, DI_22_PIN);

            CORE_AVR_PIN_SET_INPUT(DI_23_PORT, DI_23_PIN);
            CORE_AVR_PIN_SET_HIGH(DI_23_PORT, DI_23_PIN);

            CORE_AVR_PIN_SET_INPUT(DI_24_PORT, DI_24_PIN);
            CORE_AVR_PIN_SET_HIGH(DI_24_PORT, DI_24_PIN);

            CORE_AVR_PIN_SET_INPUT(DI_25_PORT, DI_25_PIN);
            CORE_AVR_PIN_SET_HIGH(DI_25_PORT, DI_25_PIN);

            CORE_AVR_PIN_SET_INPUT(DI_26_PORT, DI_26_PIN);
            CORE_AVR_PIN_SET_HIGH(DI_26_PORT, DI_26_PIN);

            CORE_AVR_PIN_SET_INPUT(DI_27_PORT, DI_27_PIN);
            CORE_AVR_PIN_SET_HIGH(DI_27_PORT, DI_27_PIN);

            CORE_AVR_PIN_SET_INPUT(DI_28_PORT, DI_28_PIN);
            CORE_AVR_PIN_SET_HIGH(DI_28_PORT, DI_28_PIN);

            CORE_AVR_PIN_SET_INPUT(DI_29_PORT, DI_29_PIN);
            CORE_AVR_PIN_SET_HIGH(DI_29_PORT, DI_29_PIN);

            CORE_AVR_PIN_SET_INPUT(DI_30_PORT, DI_30_PIN);
            CORE_AVR_PIN_SET_HIGH(DI_30_PORT, DI_30_PIN);

            CORE_AVR_PIN_SET_OUTPUT(MUX_S0_PORT, MUX_S0_PIN);
            CORE_AVR_PIN_SET_LOW(MUX_S0_PORT, MUX_S0_PIN);

            CORE_AVR_PIN_SET_OUTPUT(MUX_S1_PORT, MUX_S1_PIN);
            CORE_AVR_PIN_SET_LOW(MUX_S1_PORT, MUX_S1_PIN);

            CORE_AVR_PIN_SET_OUTPUT(MUX_S2_PORT, MUX_S2_PIN);
            CORE_AVR_PIN_SET_LOW(MUX_S2_PORT, MUX_S2_PIN);

            CORE_AVR_PIN_SET_OUTPUT(MUX_S3_PORT, MUX_S3_PIN);
            CORE_AVR_PIN_SET_LOW(MUX_S3_PORT, MUX_S3_PIN);

            CORE_AVR_PIN_SET_OUTPUT(DO_1_PORT, DO_1_PIN);
            EXT_LED_OFF(DO_1_PORT, DO_1_PIN);

            CORE_AVR_PIN_SET_OUTPUT(DO_2_PORT, DO_2_PIN);
            EXT_LED_OFF(DO_2_PORT, DO_2_PIN);

            CORE_AVR_PIN_SET_OUTPUT(DO_3_PORT, DO_3_PIN);
            EXT_LED_OFF(DO_3_PORT, DO_3_PIN);

            CORE_AVR_PIN_SET_OUTPUT(DO_4_PORT, DO_4_PIN);
            EXT_LED_OFF(DO_4_PORT, DO_4_PIN);

            CORE_AVR_PIN_SET_OUTPUT(DO_5_PORT, DO_5_PIN);
            EXT_LED_OFF(DO_5_PORT, DO_5_PIN);

            CORE_AVR_PIN_SET_OUTPUT(DO_6_PORT, DO_6_PIN);
            EXT_LED_OFF(DO_6_PORT, DO_6_PIN);

            CORE_AVR_PIN_SET_OUTPUT(DO_7_PORT, DO_7_PIN);
            EXT_LED_OFF(DO_7_PORT, DO_7_PIN);

            CORE_AVR_PIN_SET_OUTPUT(DO_8_PORT, DO_8_PIN);
            EXT_LED_OFF(DO_8_PORT, DO_8_PIN);

            CORE_AVR_PIN_SET_OUTPUT(DO_9_PORT, DO_9_PIN);
            EXT_LED_OFF(DO_9_PORT, DO_9_PIN);

            CORE_AVR_PIN_SET_OUTPUT(DO_10_PORT, DO_10_PIN);
            EXT_LED_OFF(DO_10_PORT, DO_10_PIN);

            CORE_AVR_PIN_SET_OUTPUT(DO_11_PORT, DO_11_PIN);
            EXT_LED_OFF(DO_11_PORT, DO_11_PIN);

            CORE_AVR_PIN_SET_OUTPUT(DO_12_PORT, DO_12_PIN);
            EXT_LED_OFF(DO_12_PORT, DO_12_PIN);

            CORE_AVR_PIN_SET_OUTPUT(DO_13_PORT, DO_13_PIN);
            EXT_LED_OFF(DO_13_PORT, DO_13_PIN);

            CORE_AVR_PIN_SET_OUTPUT(DO_14_PORT, DO_14_PIN);
            EXT_LED_OFF(DO_14_PORT, DO_14_PIN);

            CORE_AVR_PIN_SET_OUTPUT(DO_15_PORT, DO_15_PIN);
            EXT_LED_OFF(DO_15_PORT, DO_15_PIN);

            CORE_AVR_PIN_SET_OUTPUT(DO_16_PORT, DO_16_PIN);
            EXT_LED_OFF(DO_16_PORT, DO_16_PIN);

            CORE_AVR_PIN_SET_OUTPUT(DO_17_PORT, DO_17_PIN);
            EXT_LED_OFF(DO_17_PORT, DO_17_PIN);

            CORE_AVR_PIN_SET_OUTPUT(DO_18_PORT, DO_18_PIN);
            EXT_LED_OFF(DO_18_PORT, DO_18_PIN);

            CORE_AVR_PIN_SET_OUTPUT(DO_19_PORT, DO_19_PIN);
            EXT_LED_OFF(DO_19_PORT, DO_19_PIN);

            CORE_AVR_PIN_SET_OUTPUT(DO_20_PORT, DO_20_PIN);
            EXT_LED_OFF(DO_20_PORT, DO_20_PIN);

            CORE_AVR_PIN_SET_OUTPUT(DO_21_PORT, DO_21_PIN);
            EXT_LED_OFF(DO_21_PORT, DO_21_PIN);

            CORE_AVR_PIN_SET_OUTPUT(DO_22_PORT, DO_22_PIN);
            EXT_LED_OFF(DO_22_PORT, DO_22_PIN);

            CORE_AVR_PIN_SET_OUTPUT(DO_23_PORT, DO_23_PIN);
            EXT_LED_OFF(DO_23_PORT, DO_23_PIN);

            CORE_AVR_PIN_SET_OUTPUT(DO_24_PORT, DO_24_PIN);
            EXT_LED_OFF(DO_24_PORT, DO_24_PIN);

            CORE_AVR_PIN_SET_INPUT(MUX_1_IN_PORT, MUX_1_IN_PIN);
            CORE_AVR_PIN_SET_LOW(MUX_1_IN_PORT, MUX_1_IN_PIN);

            CORE_AVR_PIN_SET_INPUT(MUX_2_IN_PORT, MUX_2_IN_PIN);
            CORE_AVR_PIN_SET_LOW(MUX_2_IN_PORT, MUX_2_IN_PIN);

            CORE_AVR_PIN_SET_INPUT(MUX_3_IN_PORT, MUX_3_IN_PIN);
            CORE_AVR_PIN_SET_LOW(MUX_3_IN_PORT, MUX_3_IN_PIN);

            CORE_AVR_PIN_SET_INPUT(MUX_4_IN_PORT, MUX_4_IN_PIN);
            CORE_AVR_PIN_SET_LOW(MUX_4_IN_PORT, MUX_4_IN_PIN);

            CORE_AVR_PIN_SET_INPUT(MUX_5_IN_PORT, MUX_5_IN_PIN);
            CORE_AVR_PIN_SET_LOW(MUX_5_IN_PORT, MUX_5_IN_PIN);

            CORE_AVR_PIN_SET_INPUT(MUX_6_IN_PORT, MUX_6_IN_PIN);
            CORE_AVR_PIN_SET_LOW(MUX_6_IN_PORT, MUX_6_IN_PIN);
        }

        void adc()
        {
            core::CORE_ARCH::adc::conf_t adcConfiguration;

            adcConfiguration.prescaler = core::CORE_ARCH::adc::prescaler_t::p128;
            adcConfiguration.vref = core::CORE_ARCH::adc::vRef_t::avcc;

            core::CORE_ARCH::adc::setup(adcConfiguration);
            core::CORE_ARCH::adc::setChannel(Board::map::adcChannel(0));

            for (int i = 0; i < 3; i++)
                core::CORE_ARCH::adc::read();    //few dummy reads to init ADC

            core::CORE_ARCH::adc::enableInterrupt();
            core::CORE_ARCH::adc::startConversion();
        }

        void timers()
        {
            //clear timer0 conf
            TCCR0A = 0;
            TCCR0B = 0;
            TIMSK0 = 0;

            //clear timer1 conf
            TCCR1A = 0;
            TCCR1B = 0;

            //clear timer3 conf
            TCCR3A = 0;
            TCCR3B = 0;

            //set timer0 to ctc, used for millis/led matrix
            TCCR0A |= (1 << WGM01);                 //CTC mode
            TCCR0B |= (1 << CS01) | (1 << CS00);    //prescaler 64
            OCR0A = 124;                            //500us
            TIMSK0 |= (1 << OCIE0A);                //compare match interrupt
        }
    }    // namespace setup

    void ledFlashStartup(bool fwUpdated)
    {
        //there are no indicator leds on atmega2560
        //instead, send special command to USB link which will display indicator led animation

        MIDI::USBMIDIpacket_t packet;

        if (fwUpdated)
            packet.Event = static_cast<uint8_t>(OpenDeckMIDIformat::command_t::fwUpdated);
        else
            packet.Event = static_cast<uint8_t>(OpenDeckMIDIformat::command_t::fwNotUpdated);

        packet.Data1 = 0x00;
        packet.Data2 = 0x00;
        packet.Data3 = 0x00;

        OpenDeckMIDIformat::write(UART_USB_LINK_CHANNEL, packet, OpenDeckMIDIformat::packetType_t::internalCommand);
    }
}    // namespace Board