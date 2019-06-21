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
#include "../../../../../common/OpenDeckMIDIformat/OpenDeckMIDIformat.h"

namespace Board
{
    namespace setup
    {
        void pins()
        {
            setInput(DI_1_PORT, DI_1_PIN);
            setHigh(DI_1_PORT, DI_1_PIN);

            setInput(DI_2_PORT, DI_2_PIN);
            setHigh(DI_2_PORT, DI_2_PIN);

            setInput(DI_3_PORT, DI_3_PIN);
            setHigh(DI_3_PORT, DI_3_PIN);

            setInput(DI_4_PORT, DI_4_PIN);
            setHigh(DI_4_PORT, DI_4_PIN);

            setInput(DI_5_PORT, DI_5_PIN);
            setHigh(DI_5_PORT, DI_5_PIN);

            setInput(DI_6_PORT, DI_6_PIN);
            setHigh(DI_6_PORT, DI_6_PIN);

            setInput(DI_7_PORT, DI_7_PIN);
            setHigh(DI_7_PORT, DI_7_PIN);

            setInput(DI_8_PORT, DI_8_PIN);
            setHigh(DI_8_PORT, DI_8_PIN);

            setInput(DI_9_PORT, DI_9_PIN);
            setHigh(DI_9_PORT, DI_9_PIN);

            setInput(DI_10_PORT, DI_10_PIN);
            setHigh(DI_10_PORT, DI_10_PIN);

            setInput(DI_11_PORT, DI_11_PIN);
            setHigh(DI_11_PORT, DI_11_PIN);

            setInput(DI_12_PORT, DI_12_PIN);
            setHigh(DI_12_PORT, DI_12_PIN);

            setInput(DI_13_PORT, DI_13_PIN);
            setHigh(DI_13_PORT, DI_13_PIN);

            setInput(DI_14_PORT, DI_14_PIN);
            setHigh(DI_14_PORT, DI_14_PIN);

            setInput(DI_15_PORT, DI_15_PIN);
            setHigh(DI_15_PORT, DI_15_PIN);

            setInput(DI_16_PORT, DI_16_PIN);
            setHigh(DI_16_PORT, DI_16_PIN);

            setInput(DI_17_PORT, DI_17_PIN);
            setHigh(DI_17_PORT, DI_17_PIN);

            setInput(DI_18_PORT, DI_18_PIN);
            setHigh(DI_18_PORT, DI_18_PIN);

            setInput(DI_19_PORT, DI_19_PIN);
            setHigh(DI_19_PORT, DI_19_PIN);

            setInput(DI_20_PORT, DI_20_PIN);
            setHigh(DI_20_PORT, DI_20_PIN);

            setInput(DI_21_PORT, DI_21_PIN);
            setHigh(DI_21_PORT, DI_21_PIN);

            setInput(DI_22_PORT, DI_22_PIN);
            setHigh(DI_22_PORT, DI_22_PIN);

            setInput(DI_23_PORT, DI_23_PIN);
            setHigh(DI_23_PORT, DI_23_PIN);

            setInput(DI_24_PORT, DI_24_PIN);
            setHigh(DI_24_PORT, DI_24_PIN);

            setInput(DI_25_PORT, DI_25_PIN);
            setHigh(DI_25_PORT, DI_25_PIN);

            setInput(DI_26_PORT, DI_26_PIN);
            setHigh(DI_26_PORT, DI_26_PIN);

            setInput(DI_27_PORT, DI_27_PIN);
            setHigh(DI_27_PORT, DI_27_PIN);

            setInput(DI_28_PORT, DI_28_PIN);
            setHigh(DI_28_PORT, DI_28_PIN);

            setInput(DI_29_PORT, DI_29_PIN);
            setHigh(DI_29_PORT, DI_29_PIN);

            setInput(DI_30_PORT, DI_30_PIN);
            setHigh(DI_30_PORT, DI_30_PIN);

            setOutput(MUX_S0_PORT, MUX_S0_PIN);
            setLow(MUX_S0_PORT, MUX_S0_PIN);

            setOutput(MUX_S1_PORT, MUX_S1_PIN);
            setLow(MUX_S1_PORT, MUX_S1_PIN);

            setOutput(MUX_S2_PORT, MUX_S2_PIN);
            setLow(MUX_S2_PORT, MUX_S2_PIN);

            setOutput(MUX_S3_PORT, MUX_S3_PIN);
            setLow(MUX_S3_PORT, MUX_S3_PIN);

            setOutput(DO_1_PORT, DO_1_PIN);
            EXT_LED_OFF(DO_1_PORT, DO_1_PIN);

            setOutput(DO_2_PORT, DO_2_PIN);
            EXT_LED_OFF(DO_2_PORT, DO_2_PIN);

            setOutput(DO_3_PORT, DO_3_PIN);
            EXT_LED_OFF(DO_3_PORT, DO_3_PIN);

            setOutput(DO_4_PORT, DO_4_PIN);
            EXT_LED_OFF(DO_4_PORT, DO_4_PIN);

            setOutput(DO_5_PORT, DO_5_PIN);
            EXT_LED_OFF(DO_5_PORT, DO_5_PIN);

            setOutput(DO_6_PORT, DO_6_PIN);
            EXT_LED_OFF(DO_6_PORT, DO_6_PIN);

            setOutput(DO_7_PORT, DO_7_PIN);
            EXT_LED_OFF(DO_7_PORT, DO_7_PIN);

            setOutput(DO_8_PORT, DO_8_PIN);
            EXT_LED_OFF(DO_8_PORT, DO_8_PIN);

            setOutput(DO_9_PORT, DO_9_PIN);
            EXT_LED_OFF(DO_9_PORT, DO_9_PIN);

            setOutput(DO_10_PORT, DO_10_PIN);
            EXT_LED_OFF(DO_10_PORT, DO_10_PIN);

            setOutput(DO_11_PORT, DO_11_PIN);
            EXT_LED_OFF(DO_11_PORT, DO_11_PIN);

            setOutput(DO_12_PORT, DO_12_PIN);
            EXT_LED_OFF(DO_12_PORT, DO_12_PIN);

            setOutput(DO_13_PORT, DO_13_PIN);
            EXT_LED_OFF(DO_13_PORT, DO_13_PIN);

            setOutput(DO_14_PORT, DO_14_PIN);
            EXT_LED_OFF(DO_14_PORT, DO_14_PIN);

            setOutput(DO_15_PORT, DO_15_PIN);
            EXT_LED_OFF(DO_15_PORT, DO_15_PIN);

            setOutput(DO_16_PORT, DO_16_PIN);
            EXT_LED_OFF(DO_16_PORT, DO_16_PIN);

            setOutput(DO_17_PORT, DO_17_PIN);
            EXT_LED_OFF(DO_17_PORT, DO_17_PIN);

            setOutput(DO_18_PORT, DO_18_PIN);
            EXT_LED_OFF(DO_18_PORT, DO_18_PIN);

            setOutput(DO_19_PORT, DO_19_PIN);
            EXT_LED_OFF(DO_19_PORT, DO_19_PIN);

            setOutput(DO_20_PORT, DO_20_PIN);
            EXT_LED_OFF(DO_20_PORT, DO_20_PIN);

            setOutput(DO_21_PORT, DO_21_PIN);
            EXT_LED_OFF(DO_21_PORT, DO_21_PIN);

            setOutput(DO_22_PORT, DO_22_PIN);
            EXT_LED_OFF(DO_22_PORT, DO_22_PIN);

            setOutput(DO_23_PORT, DO_23_PIN);
            EXT_LED_OFF(DO_23_PORT, DO_23_PIN);

            setOutput(DO_24_PORT, DO_24_PIN);
            EXT_LED_OFF(DO_24_PORT, DO_24_PIN);

            setInput(MUX_1_IN_PORT, MUX_1_IN_PIN);
            setLow(MUX_1_IN_PORT, MUX_1_IN_PIN);

            setInput(MUX_2_IN_PORT, MUX_2_IN_PIN);
            setLow(MUX_2_IN_PORT, MUX_2_IN_PIN);

            setInput(MUX_3_IN_PORT, MUX_3_IN_PIN);
            setLow(MUX_3_IN_PORT, MUX_3_IN_PIN);

            setInput(MUX_4_IN_PORT, MUX_4_IN_PIN);
            setLow(MUX_4_IN_PORT, MUX_4_IN_PIN);

            setInput(MUX_5_IN_PORT, MUX_5_IN_PIN);
            setLow(MUX_5_IN_PORT, MUX_5_IN_PIN);

            setInput(MUX_6_IN_PORT, MUX_6_IN_PIN);
            setLow(MUX_6_IN_PORT, MUX_6_IN_PIN);
        }

        void adc()
        {
            using namespace core::avr;

            adc::conf_t adcConfiguration;

            adcConfiguration.prescaler = adc::prescaler_t::p128;
            adcConfiguration.vref = adc::vRef_t::avcc;

            adc::setup(adcConfiguration);
            adc::setChannel(Board::map::adcChannel(0));

            for (int i = 0; i < 3; i++)
                adc::read();    //few dummy reads to init ADC

            adc::enableInterrupt();
            adc::startConversion();
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
            packet.Event = static_cast<uint8_t>(odFormatCMD_t::cmdFwUpdated);
        else
            packet.Event = static_cast<uint8_t>(odFormatCMD_t::cmdFwNotUpdated);

        packet.Data1 = 0x00;
        packet.Data2 = 0x00;
        packet.Data3 = 0x00;

        OpenDeckMIDIformat::write(UART_USB_LINK_CHANNEL, packet, odPacketType_t::packetIntCMD);
    }
}    // namespace Board