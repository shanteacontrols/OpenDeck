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

#pragma once

#include <inttypes.h>
#include "midi/src/MIDI.h"
#include "core/src/general/IO.h"

#ifdef __STM32__
#include "board/stm32/eeprom/EEPROM.h"
#endif

//for internal board usage only - do not include/call in application directly

namespace Board
{
    namespace detail
    {
        ///
        /// \brief Specifies incoming or outgoing MIDI data traffic.
        ///
        enum class midiTrafficDirection_t : uint8_t
        {
            incoming,
            outgoing
        };

        namespace setup
        {
            ///
            /// \brief Initializes all used clocks on the board.
            ///
            void clocks();

            ///
            /// \brief Initializes all pins to correct states.
            ///
            void io();

#if !defined(OD_BOARD_16U2) && !defined(OD_BOARD_8U2)
            ///
            /// \brief Initializes analog variables and ADC peripheral.
            ///
            void adc();
#endif

#ifdef USB_MIDI_SUPPORTED
            ///
            /// \brief Initializes USB peripheral and configures it as MIDI device.
            ///
            void usb();
#endif

            ///
            /// \brief Initializes all used timers on board.
            ///
            void timers();
        }    // namespace setup

        namespace UART
        {
            namespace ll
            {
                //low-level UART API, MCU specific

                ///
                /// \brief Enables the firing of interrupt once the UART data register is empty.
                /// This effectively starts the process of transmitting the data from UART TX buffer to UART interface.
                /// @param [in] channel     UART channel on MCU.
                ///
                void enableDataEmptyInt(uint8_t channel);

                ///
                /// \brief Disables the firing of interrupt once the UART data register is empty.
                /// @param [in] channel     UART channel on MCU.
                ///
                void disableDataEmptyInt(uint8_t channel);

                ///
                /// \brief Performs low-level initialization of the specified UART channel.
                /// @param [in] channel     UART channel on MCU.
                /// @param [in] baudRate    UART speed (baudrate).
                ///
                void init(uint8_t channel, uint32_t baudRate);

                ///
                /// \brief Performs low-level deinitialization of the specified UART channel.
                /// @param [in] channel UART channel on MCU.
                ///
                void deInit(uint8_t channel);

                ///
                /// \brief Performs direct writing of data to outgoing UART register.
                /// @param [in] channel UART channel on MCU.
                /// @param [in] data    Data to write.
                ///
                void directWrite(uint8_t channel, uint8_t data);
            }    // namespace ll

            ///
            /// \brief Global ISR handler for all UART events.
            /// @param [in] channel UART channel on MCU.
            ///
            void isrHandler(uint8_t channel);

            ///
            /// \brief Used to store incoming data from UART to buffer.
            /// @param [in] channel UART channel on MCU.
            /// @param [in] data    Received data.
            ///
            void storeIncomingData(uint8_t channel, uint8_t data);

            ///
            /// \brief Retrieves the next byte from the outgoing ring buffer.
            /// @param [in] channel UART channel on MCU.
            /// @param [in] data    Data to send.
            ///
            bool getNextByteToSend(uint8_t channel, uint8_t& data);

            ///
            /// \brief Used to indicate that the transmission is complete.
            /// @param [in] channel UART channel on MCU.
            ///
            void indicateTxComplete(uint8_t channel);
        }    // namespace UART

        namespace map
        {
            ///
            /// \brief Used to retrieve real analog multiplexer channel for an given index.
            ///
            uint8_t muxChannel(uint8_t index);

            ///
            /// \brief Used to retrieve real row address in an input matrix for an given index.
            ///
            uint8_t inMatrixRow(uint8_t index);

            ///
            /// \brief Used to retrieve real column address in an input matrix for an given index.
            ///
            uint8_t inMatrixColumn(uint8_t index);

            ///
            /// \brief Used to retrieve ADC port and pin for a given ADC channel.
            ///
            core::io::mcuPin_t adcChannel(uint8_t index);

            ///
            /// \brief Used to retrieve button port and pin for a given button index.
            ///
            core::io::mcuPin_t button(uint8_t index);

            ///
            /// \brief Used to retrieve LED port and pin for a given LED index.
            ///
            core::io::mcuPin_t led(uint8_t index);

            ///
            /// \brief Used to retrieve all the registers needed to control PWM channel for an given index.
            ///
            core::io::pwmChannel_t pwmChannel(uint8_t index);

#ifdef __STM32__
            ///
            /// \brief Used to retrieve actual UART interface on board for a given UART channel index.
            ///
            USART_TypeDef* uartInterface(uint8_t channel);

            ///
            /// \brief Used to retrieve timer instance used for main timer interrupt.
            ///
            TIM_TypeDef* mainTimerInstance();

            ///
            /// \brief Used to retrieve descriptors for flash pages used for EEPROM emulation.
            /// @ {

            EmuEEPROM::pageDescriptor_t& eepromFlashPage1();
            EmuEEPROM::pageDescriptor_t& eepromFlashPage2();

            /// }
#endif
        }    // namespace map

        namespace io
        {
            ///
            /// \brief Called in ADC ISR once the conversion is done.
            /// @param [in] adcValue    Retrieved ADC value.
            ///
            void adcISRHandler(uint16_t adcValue);

            ///
            /// \brief Continuously reads all digital inputs.
            ///
            void checkDigitalInputs();

            ///
            /// \brief Checks if digital outputs need to be updated (state and PWM control).
            ///
            void checkDigitalOutputs();

#ifdef LED_INDICATORS
            ///
            /// \brief Used to indicate that the MIDI event has occured using built-in LEDs on board.
            /// @param [source]     Source of MIDI data. See MIDI::interface_t enumeration.
            /// @param [direction]  Direction of MIDI data. See midiTrafficDirection_t enumeration.
            ///
            void indicateMIDItraffic(MIDI::interface_t source, midiTrafficDirection_t direction);

            ///
            /// \brief Checks if indicator LEDs need to be turned on or off.
            ///
            void checkIndicators();

            ///
            /// \brief Enables the checking of LED indicators in ISR.
            ///
            void enableIndicators();

            ///
            /// \brief Disables the checking of LED indicators in ISR.
            ///
            void disableIndicators();
#endif

#ifdef FW_BOOT
            ///
            /// \brief Reads the state of the button responsible for hardware bootloader entry.
            /// \returns True if pressed, false otherwise. If bootloader button doesn't exist,
            ///          function will return false.
            ///
            bool isBtldrButtonActive();

            ///
            /// \brief Initializes outputs used to indicate that bootloader mode is active.
            ///
            void indicateBtldr();
#endif
        }    // namespace io
    }        // namespace detail
}    // namespace Board