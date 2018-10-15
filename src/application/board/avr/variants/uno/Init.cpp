/*
    OpenDeck MIDI platform firmware
    Copyright (C) 2015-2018 Igor Petrovic

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "board/Board.h"
#include "pins/Pins.h"
#include "board/common/constants/LEDs.h"
#include "board/common/uart/ODformat.h"
#include "core/src/HAL/avr/PinManipulation.h"
#include "core/src/HAL/avr/adc/ADC.h"

namespace Board
{
    namespace detail
    {
        void initPins()
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


            setInput(AI_1_PORT, AI_1_PIN);
            setLow(AI_1_PORT, AI_1_PIN);

            setInput(AI_2_PORT, AI_2_PIN);
            setLow(AI_2_PORT, AI_2_PIN);

            setInput(AI_3_PORT, AI_3_PIN);
            setLow(AI_3_PORT, AI_3_PIN);

            setInput(AI_4_PORT, AI_4_PIN);
            setLow(AI_4_PORT, AI_4_PIN);

            setInput(AI_5_PORT, AI_5_PIN);
            setLow(AI_5_PORT, AI_5_PIN);

            setInput(AI_6_PORT, AI_6_PIN);
            setLow(AI_6_PORT, AI_6_PIN);
        }

        void initAnalog()
        {
            adcConf adcConfiguration;

            adcConfiguration.prescaler = ADC_PRESCALER_128;
            adcConfiguration.vref = ADC_VREF_AVCC;

            setUpADC(adcConfiguration);
            setADCchannel(AI_1_PIN);

            for (int i=0; i<3; i++)
                getADCvalue();  //few dummy reads to init ADC

            adcInterruptEnable();
            startADCconversion();
        }

        void configureTimers()
        {
            //clear timer0 conf
            TCCR0A = 0;
            TCCR0B = 0;
            TIMSK0 = 0;

            //clear timer1 conf
            TCCR1A = 0;
            TCCR1B = 0;

            //set timer0 to ctc, used for millis/led matrix
            TCCR0A |= (1<<WGM01);           //CTC mode
            TCCR0B |= (1<<CS01)|(1<<CS00);  //prescaler 64
            OCR0A = 124;                    //500us
            TIMSK0 |= (1<<OCIE0A);          //compare match interrupt
        }
    }

    void ledFlashStartup(bool fwUpdated)
    {
        using namespace Board::detail;

        //there are no indicator leds on atmega328p
        //instead, send special command to USB link which will display indicator led animation
        uartWrite(UART_USB_LINK_CHANNEL, OD_FORMAT_INT_DATA_START);

        if (fwUpdated)
            uartWrite(UART_USB_LINK_CHANNEL, static_cast<uint8_t>(odFormatCMD_t::cmdFwUpdated));
        else
            uartWrite(UART_USB_LINK_CHANNEL, static_cast<uint8_t>(odFormatCMD_t::cmdFwNotUpdated));

        uartWrite(UART_USB_LINK_CHANNEL, 0x00);
        uartWrite(UART_USB_LINK_CHANNEL, 0x00);
        uartWrite(UART_USB_LINK_CHANNEL, 0x00);
        uartWrite(UART_USB_LINK_CHANNEL, 0x00);
    }

    bool startUpAnimation()
    {
        return false;
    }
}