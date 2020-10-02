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

#pragma once

#include <inttypes.h>
#include "midi/src/MIDI.h"
#include "core/src/general/IO.h"

#ifdef __STM32__
#include "EmuEEPROM/src/EmuEEPROM.h"
#endif

#ifndef __AVR__
#include <vector>
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

        ///
        /// \brief Default error handler.
        ///
        void errorHandler();

        namespace setup
        {
            ///
            /// \brief Prepares MCU to run application.
            ///
            void application();

            ///
            /// \brief Prepares MCU to run bootloader.
            ///
            void bootloader();

            ///
            /// \brief Initializes all used clocks on the board.
            ///
            void clocks();

            ///
            /// \brief Initializes all pins to correct states.
            ///
            void io();

#ifndef USB_LINK_MCU
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
                bool init(uint8_t channel, uint32_t baudRate);

                ///
                /// \brief Performs low-level deinitialization of the specified UART channel.
                /// @param [in] channel UART channel on MCU.
                ///
                bool deInit(uint8_t channel);

                ///
                /// \brief Performs direct writing of data to outgoing UART register.
                /// @param [in] channel UART channel on MCU.
                /// @param [in] data    Data to write.
                ///
                void directWrite(uint8_t channel, uint8_t data);
            }    // namespace ll

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
            typedef struct
            {
                core::io::mcuPin_t miso;
                core::io::mcuPin_t mosi;
                core::io::mcuPin_t sck;
            } SPIpins_t;

            typedef struct
            {
                core::io::mcuPin_t sda;
                core::io::mcuPin_t sdl;
            } I2Cpins_t;

            typedef struct
            {
                uint32_t address;
                uint32_t size;
            } flashPage_t;

            ///
            /// \brief Used to retrieve physical ADC channel for a given MCU pin.
            ///
            uint32_t adcChannel(core::io::mcuPin_t pin);

            ///
            /// \brief Used to retrieve physical ADC channel for a given ADC channel index.
            ///
            uint32_t adcChannel(uint8_t index);

            ///
            /// \brief Used to retrieve ADC port and pin for a given ADC channel index.
            ///
            core::io::mcuPin_t adcPin(uint8_t index);

            ///
            /// \brief Used to retrieve physical analog component index for a given user-specified index.
            ///
            uint8_t adcIndex(uint8_t index);

            ///
            /// \brief Used to retrieve button port and pin for a given button index.
            ///
            core::io::mcuPin_t buttonPin(uint8_t index);

            ///
            /// \brief Used to retrieve physical button component index for a given user-specified index.
            ///
            uint8_t buttonIndex(uint8_t index);

            ///
            /// \brief Used to retrieve LED port and pin for a given LED index.
            ///
            core::io::mcuPin_t ledPin(uint8_t index);

            ///
            /// \brief Used to physical LED component index for a given user-specified index.
            ///
            uint8_t ledIndex(uint8_t index);

            ///
            /// \brief Used to retrieve all the registers needed to control PWM channel for an given index.
            ///
            core::io::pwmChannel_t pwmChannel(uint8_t index);

            ///
            /// \brief Retrieves flash page descriptor containing page address and size.
            /// @param [in] pageIndex Index of flash sector for which to retrieve address and size.
            /// \returns Reference to flash page descriptor for specified page index.
            ///
            flashPage_t& flashPageDescriptor(size_t pageIndex);

            ///
            /// \brief Used to retrieve descriptors for flash pages used for EEPROM emulation.
            /// @ {

            size_t eepromFlashPageFactory();
            size_t eepromFlashPage1();
            size_t eepromFlashPage2();

            /// }

#ifdef __STM32__
            class STMPeripheral
            {
                public:
                STMPeripheral() = default;

                virtual std::vector<core::io::mcuPin_t> pins()         = 0;
                virtual void*                           interface()    = 0;
                virtual IRQn_Type                       irqn()         = 0;
                virtual void                            enableClock()  = 0;
                virtual void                            disableClock() = 0;
            };

            ///
            /// Used to retrieve physical UART interface used on MCU for a given UART channel index as well
            /// as pins on which the interface is connected.
            ///
            STMPeripheral* uartDescriptor(uint8_t channel);

            ///
            /// Used to retrieve physical I2C interface used on MCU for a given I2C channel index as well
            /// as pins on which the interface is connected.
            ///
            STMPeripheral* i2cDescriptor(uint8_t channel);

            ///
            /// \brief Used to retrieve UART channel on board for a specified UART interface.
            /// If no channels are mapped to the provided interface, return false.
            ///
            bool uartChannel(USART_TypeDef* interface, uint8_t& channel);

            ///
            /// \brief Used to retrieve I2C channel on board for a specified UART interface.
            /// If no channels are mapped to the provided interface, return false.
            ///
            bool i2cChannel(I2C_TypeDef* interface, uint8_t& channel);

            ///
            /// \brief Used to retrieve physical ADC interface used on MCU.
            ///
            ADC_TypeDef* adcInterface();

            ///
            /// \brief Used to retrieve timer instance used for main timer interrupt.
            ///
            TIM_TypeDef* mainTimerInstance();
#endif
        }    // namespace map

        namespace io
        {
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

            ///
            /// \brief Flashes integrated LEDs on board on startup.
            /// Pattern differs depending on whether firmware is updated or not.
            /// @param[in] fwUpdated    If set to true, "Firmware updated" pattern will be
            ///                         used to flash the LEDs.
            ///
            void ledFlashStartup(bool fwUpdated);
#endif

            ///
            /// \brief MCU-specific delay routine used when setting 74HC595 shift register state.
            ///
            void sr595wait();

            ///
            /// \brief MCU-specific delay routine used when setting 74HC165 shift register state.
            ///
            void sr165wait();

            ///
            /// \brief Used to temporarily configure all common multiplexer pins as outputs to minimize
            /// the effect of channel-to-channel crosstalk.
            ///
            void dischargeMux();

            ///
            /// \brief Used to restore pin setup for specified multiplexer.
            ///
            void restoreMux(uint8_t muxIndex);
        }    // namespace io

        namespace isrHandling
        {
            ///
            /// \brief Global ISR handler for all UART events.
            /// @param [in] channel UART channel on MCU.
            ///
            void uart(uint8_t channel);

            ///
            /// \brief Called in ADC ISR once the conversion is done.
            /// @param [in] adcValue    Retrieved ADC value.
            ///
            void adc(uint16_t adcValue);

            ///
            /// \brief Global ISR handler for main timer.
            ///
            void mainTimer();
        }    // namespace isrHandling

        namespace bootloader
        {
            ///
            /// \brief List of all possible bootloader triggers.
            ///
            enum class btldrTrigger_t : uint8_t
            {
                software,
                hardware,
                all,
                none
            };

            ///
            /// \brief Verifies if the programmed flash is valid.
            /// \return True if valid, false otherwise.
            ///
            bool isAppValid();

            ///
            /// \brief Checks if any of the bootloader entry triggers are active.
            ///
            btldrTrigger_t btldrTrigger();

            ///
            /// \brief Checks if bootloader mode should be triggered because of software trigger.
            /// \returns True if bootloader mode was triggered from application, false otherwise.
            ///
            bool isSWtriggerActive();

            ///
            /// \brief Reads the state of the button responsible for hardware bootloader entry.
            /// \returns True if pressed, false otherwise. If bootloader button doesn't exist,
            ///          function will return false.
            ///
            bool isHWtriggerActive();

            ///
            /// \brief Configures the MCU to run bootloader on next reset.
            ///
            void enableSWtrigger();

            ///
            /// \brief Clears configured software bootloader trigger so that bootloader
            /// isn't run on next reset.
            ///
            void clearSWtrigger();

            ///
            /// \brief Initializes outputs used to indicate that bootloader mode is active.
            ///
            void indicate();

            ///
            /// \brief Runs the bootloader.
            ///
            void runBootloader();

            ///
            /// \brief Jumps to application.
            ///
            void runApplication();
        }    // namespace bootloader

        namespace flash
        {
            ///
            /// \brief Checks whether the specified flash address is valid / in range for the current MCU.
            ///
            bool isInRange(uint32_t address);

            ///
            /// \brief Retrieves total flash size in bytes.
            ///
            uint32_t size();

            ///
            /// \brief Retrieves size of specified flash page.
            ///
            uint32_t pageSize(size_t index);

            ///
            /// \brief Erases specified flash page.
            ///
            bool erasePage(size_t index);

            ///
            /// \brief Writes filled page buffer to flash page if needed/supported.
            ///
            void writePage(size_t index);

            ///
            /// \brief Write 16-bit data to specified address in flash memory.
            ///
            bool write16(uint32_t address, uint16_t data);

            ///
            /// \brief Write 32-bit data to specified address in flash memory.
            ///
            bool write32(uint32_t address, uint32_t data);

            ///
            /// \brief Read 8-bit data from specified address in flash memory.
            ///
            bool read8(uint32_t address, uint8_t& data);

            ///
            /// \brief Read 16-bit data from specified address in flash memory.
            ///
            bool read16(uint32_t address, uint16_t& data);

            ///
            /// \brief Read 32-bit data from specified address in flash memory.
            ///
            bool read32(uint32_t address, uint32_t& data);
        }    // namespace flash
    }        // namespace detail
}    // namespace Board