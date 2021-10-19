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

#include <cstddef>
#include <stdio.h>
#include <stdlib.h>
#include <avr/power.h>
#include <avr/eeprom.h>
#include <avr/wdt.h>
#include "board/Board.h"
#include "board/Internal.h"
#include "board/common/io/Helpers.h"
#include "core/src/general/ADC.h"
#include "core/src/arch/avr/Misc.h"
#include "core/src/general/IO.h"
#include "core/src/general/Interrupt.h"
#include "core/src/general/Timing.h"
#include <Pins.h>

// serial numbers are available only on AVR MCUs with USB
#ifdef USB_SUPPORTED
#include "LUFA/Drivers/USB/USB.h"
#endif

extern "C" void __cxa_pure_virtual()
{
    Board::detail::errorHandler();
}

void* operator new(std::size_t size)
{
    return malloc(size);
}

void operator delete(void* ptr)
{
}

void operator delete(void* ptr, unsigned int _s)
{
}

namespace std
{
    void __throw_bad_function_call()
    {
        Board::detail::errorHandler();

        while (1)
            ;
    }

    void __throw_bad_alloc()
    {
        Board::detail::errorHandler();

        while (1)
            ;
    }

    void __throw_out_of_range(char const* arg)
    {
        Board::detail::errorHandler();

        while (1)
            ;
    }
}    // namespace std

namespace Board
{
#ifdef USB_SUPPORTED
    void uniqueID(uniqueID_t& uid)
    {
        ATOMIC_SECTION
        {
            uint8_t address = INTERNAL_SERIAL_START_ADDRESS;

            for (uint8_t i = 0; i < (UID_BITS / 8); i++)
            {
                uid[i] = boot_signature_byte_get(address++);

                // LUFA sends unique ID with nibbles swaped
                // to match with LUFA, invert them here
                uid[i] = (uid[i] << 4) | ((uid[i] >> 4) & 0x0F);
            }
        }
    }
#endif

    namespace detail
    {
        namespace setup
        {
            void bootloader()
            {
                DISABLE_INTERRUPTS();

                // clear reset source
                MCUSR &= ~(1 << EXTRF);

                // disable watchdog
                MCUSR &= ~(1 << WDRF);
                wdt_disable();

                // disable clock division
                clock_prescale_set(clock_div_1);

                detail::setup::io();

                // relocate the interrupt vector table to the bootloader section
                MCUCR = (1 << IVCE);
                MCUCR = (1 << IVSEL);

                ENABLE_INTERRUPTS();

#if defined(USB_LINK_MCU) || !defined(USB_SUPPORTED)
                Board::UART::config_t config(UART_BAUDRATE_USB,
                                             Board::UART::parity_t::no,
                                             Board::UART::stopBits_t::one,
                                             Board::UART::type_t::rxTx);

                Board::UART::init(UART_CHANNEL_USB_LINK, config);
#endif
            }

            void application()
            {
                DISABLE_INTERRUPTS();

                // relocate the interrupt vector table to the application section
                MCUCR = (1 << IVCE);
                MCUCR = (0 << IVSEL);

                // clear reset source
                MCUSR &= ~(1 << EXTRF);

                // disable watchdog
                MCUSR &= ~(1 << WDRF);
                wdt_disable();

                // disable clock division
                clock_prescale_set(clock_div_1);

                detail::setup::io();
                detail::setup::adc();

#if defined(USB_LINK_MCU) || !defined(USB_SUPPORTED)
                Board::UART::config_t config(UART_BAUDRATE_USB,
                                             Board::UART::parity_t::no,
                                             Board::UART::stopBits_t::one,
                                             Board::UART::type_t::rxTx);

                Board::UART::init(UART_CHANNEL_USB_LINK, config);
#endif

                detail::setup::usb();
                detail::setup::timers();

                ENABLE_INTERRUPTS();

#ifndef USB_LINK_MCU
                // add some delay and remove initial readout of digital inputs
                core::timing::waitMs(10);
                detail::io::flushInputReadings();

#ifndef USB_SUPPORTED
                // wait for unique id from usb host
                // this is to make sure host and the device share the same unique id
                USBLink::internalCMD_t cmd;

                while (1)
                {
                    while (!Board::detail::USB::readInternal(cmd))
                        ;

                    if (cmd == USBLink::internalCMD_t::uniqueID)
                        break;
                }
#endif
#endif
            }

#ifdef ANALOG_SUPPORTED
            void adc()
            {
                core::adc::conf_t adcConfiguration;

                adcConfiguration.prescaler = core::adc::prescaler_t::p128;

#ifdef ADC_EXT_REF
                adcConfiguration.vref = core::adc::vRef_t::aref;
#else
                adcConfiguration.vref = core::adc::vRef_t::avcc;
#endif

                for (int i = 0; i < MAX_ADC_CHANNELS; i++)
                    core::adc::disconnectDigitalIn(Board::detail::map::adcChannel(i));

                core::adc::setup(adcConfiguration);
                core::adc::setChannel(Board::detail::map::adcChannel(0));

                for (int i = 0; i < 3; i++)
                    core::adc::read();    // few dummy reads to init ADC

                core::adc::enableInterrupt();
                core::adc::startConversion();
            }
#endif

            void timers()
            {
                // set timer0 to ctc, used for millis/led matrix
                // clear timer0 conf
                TCCR0A = 0;
                TCCR0B = 0;
                TIMSK0 = 0;
                TCCR0A |= (1 << WGM01);                 // CTC mode
                TCCR0B |= (1 << CS01) | (1 << CS00);    // prescaler 64
                OCR0A = 249;                            // 1ms
                TIMSK0 |= (1 << OCIE0A);                // compare match interrupt

#ifdef FW_APP
#ifndef USB_LINK_MCU
#if MAX_NUMBER_OF_LEDS > 0
                // use timer1 for soft pwm
                TCCR1A = 0;
                TCCR1B = 0;
                TCCR1C = 0;

                TIMSK1 = 0;
                TCCR1B |= (1 << WGM12);                 // CTC mode
                TCCR1B |= (1 << CS11) | (1 << CS10);    // prescaler 64
                OCR1A = 124;                            // 500us
                TIMSK1 |= (1 << OCIE1A);                // compare match interrupt
#endif
#endif
#endif
            }

            void io()
            {
#ifdef NUMBER_OF_IN_SR
                CORE_IO_CONFIG(SR_IN_DATA_PORT, SR_IN_DATA_PIN, core::io::pinMode_t::input);
                CORE_IO_CONFIG(SR_IN_CLK_PORT, SR_IN_CLK_PIN, core::io::pinMode_t::output);
                CORE_IO_CONFIG(SR_IN_LATCH_PORT, SR_IN_LATCH_PIN, core::io::pinMode_t::output);

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

                    CORE_IO_CONFIG(CORE_IO_MCU_PIN_PORT(pin), CORE_IO_MCU_PIN_INDEX(pin), core::io::pinMode_t::input);

#ifndef BUTTONS_EXT_PULLUPS
                    CORE_IO_SET_HIGH(CORE_IO_MCU_PIN_PORT(pin), CORE_IO_MCU_PIN_INDEX(pin));
#endif
                }
#endif

#ifdef NUMBER_OF_BUTTON_COLUMNS
                CORE_IO_CONFIG(DEC_BM_PORT_A0, DEC_BM_PIN_A0, core::io::pinMode_t::output);
                CORE_IO_CONFIG(DEC_BM_PORT_A1, DEC_BM_PIN_A1, core::io::pinMode_t::output);
                CORE_IO_CONFIG(DEC_BM_PORT_A2, DEC_BM_PIN_A2, core::io::pinMode_t::output);

                CORE_IO_SET_LOW(DEC_BM_PORT_A0, DEC_BM_PIN_A0);
                CORE_IO_SET_LOW(DEC_BM_PORT_A1, DEC_BM_PIN_A1);
                CORE_IO_SET_LOW(DEC_BM_PORT_A2, DEC_BM_PIN_A2);
#endif

#ifdef NUMBER_OF_OUT_SR
                CORE_IO_CONFIG(SR_OUT_DATA_PORT, SR_OUT_DATA_PIN, core::io::pinMode_t::output);
                CORE_IO_CONFIG(SR_OUT_CLK_PORT, SR_OUT_CLK_PIN, core::io::pinMode_t::output);
                CORE_IO_CONFIG(SR_OUT_LATCH_PORT, SR_OUT_LATCH_PIN, core::io::pinMode_t::output);

#ifdef SR_OUT_OE_PORT
                CORE_IO_CONFIG(SR_OUT_OE_PORT, SR_OUT_OE_PIN, core::io::pinMode_t::output);
#endif

                // init all outputs on shift register
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
                CORE_IO_CONFIG(DEC_LM_PORT_A0, DEC_LM_PIN_A0, core::io::pinMode_t::output);
                CORE_IO_CONFIG(DEC_LM_PORT_A1, DEC_LM_PIN_A1, core::io::pinMode_t::output);
                CORE_IO_CONFIG(DEC_LM_PORT_A2, DEC_LM_PIN_A2, core::io::pinMode_t::output);

                CORE_IO_SET_LOW(DEC_LM_PORT_A0, DEC_LM_PIN_A0);
                CORE_IO_SET_LOW(DEC_LM_PORT_A1, DEC_LM_PIN_A1);
                CORE_IO_SET_LOW(DEC_LM_PORT_A2, DEC_LM_PIN_A2);

                for (int i = 0; i < NUMBER_OF_LED_ROWS; i++)
#else
                for (int i = 0; i < MAX_NUMBER_OF_LEDS; i++)
#endif
                {
                    core::io::mcuPin_t pin = detail::map::ledPin(i);

                    CORE_IO_CONFIG(CORE_IO_MCU_PIN_PORT(pin), CORE_IO_MCU_PIN_INDEX(pin), core::io::pinMode_t::output);
                    EXT_LED_OFF(CORE_IO_MCU_PIN_PORT(pin), CORE_IO_MCU_PIN_INDEX(pin));
                }
#endif

#if MAX_ADC_CHANNELS > 0
                for (int i = 0; i < MAX_ADC_CHANNELS; i++)
                {
                    core::io::mcuPin_t pin = detail::map::adcPin(i);

                    CORE_IO_CONFIG(CORE_IO_MCU_PIN_PORT(pin), CORE_IO_MCU_PIN_INDEX(pin), core::io::pinMode_t::input);
                    CORE_IO_SET_LOW(CORE_IO_MCU_PIN_PORT(pin), CORE_IO_MCU_PIN_INDEX(pin));
                }
#endif

#ifdef NUMBER_OF_MUX
                CORE_IO_CONFIG(MUX_PORT_S0, MUX_PIN_S0, core::io::pinMode_t::output);
                CORE_IO_CONFIG(MUX_PORT_S1, MUX_PIN_S1, core::io::pinMode_t::output);
                CORE_IO_CONFIG(MUX_PORT_S2, MUX_PIN_S2, core::io::pinMode_t::output);
#ifdef MUX_PORT_S3
                CORE_IO_CONFIG(MUX_PORT_S3, MUX_PIN_S3, core::io::pinMode_t::output);
#endif
#endif

#ifdef BTLDR_BUTTON_PORT
                CORE_IO_CONFIG(BTLDR_BUTTON_PORT, BTLDR_BUTTON_PIN, core::io::pinMode_t::input);
#ifndef BTLDR_BUTTON_AH
                CORE_IO_SET_HIGH(BTLDR_BUTTON_PORT, BTLDR_BUTTON_PIN);
#endif
#endif

#ifdef TOTAL_UNUSED_IO
                for (int i = 0; i < TOTAL_UNUSED_IO; i++)
                {
                    Board::detail::io::unusedIO_t unusedPin = detail::map::unusedPin(i);

                    CORE_IO_CONFIG(CORE_IO_MCU_PIN_PORT(unusedPin.pin), CORE_IO_MCU_PIN_INDEX(unusedPin.pin), unusedPin.pin.mode);
                    CORE_IO_SET_STATE(CORE_IO_MCU_PIN_PORT(unusedPin.pin), CORE_IO_MCU_PIN_INDEX(unusedPin.pin), unusedPin.state);
                }
#endif

#ifdef LED_INDICATORS
                CORE_IO_CONFIG(LED_MIDI_IN_DIN_PORT, LED_MIDI_IN_DIN_PIN, core::io::pinMode_t::output);
                CORE_IO_CONFIG(LED_MIDI_OUT_DIN_PORT, LED_MIDI_OUT_DIN_PIN, core::io::pinMode_t::output);
                CORE_IO_CONFIG(LED_MIDI_IN_USB_PORT, LED_MIDI_IN_USB_PIN, core::io::pinMode_t::output);
                CORE_IO_CONFIG(LED_MIDI_OUT_USB_PORT, LED_MIDI_OUT_USB_PIN, core::io::pinMode_t::output);

                INT_LED_OFF(LED_MIDI_IN_DIN_PORT, LED_MIDI_IN_DIN_PIN);
                INT_LED_OFF(LED_MIDI_OUT_DIN_PORT, LED_MIDI_OUT_DIN_PIN);
                INT_LED_OFF(LED_MIDI_IN_USB_PORT, LED_MIDI_IN_USB_PIN);
                INT_LED_OFF(LED_MIDI_OUT_USB_PORT, LED_MIDI_OUT_USB_PIN);
#endif
            }
        }    // namespace setup
    }        // namespace detail
}    // namespace Board