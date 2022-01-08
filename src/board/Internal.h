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

#pragma once

#include <stddef.h>
#include <inttypes.h>
#include "midi/src/MIDI.h"
#include "core/src/general/IO.h"
#include "board/Board.h"
#include "usb-link/Commands.h"

// for internal board usage only - do not include/call in application directly

namespace Board
{
    namespace detail
    {
        /// Default error handler.
        void errorHandler();

        namespace setup
        {
            /// Initializes low-level layer needed for HAL API.
            void halInit();

            /// Deinitializes low-level layer needed for HAL API.
            void halDeinit();

            /// Prepares MCU to run application.
            void application();

            /// Prepares MCU to run bootloader.
            void bootloader();

            /// Initializes all used clocks on the board.
            void clocks();

            /// Initializes all pins to correct states.
            void io();

            /// Initializes analog variables and ADC peripheral.
            void adc();

            /// Initializes USB peripheral and configures it as MIDI device.
            void usb();

            /// Initializes all used timers on board.
            void timers();
        }    // namespace setup

        namespace USB
        {
            /// Used to indicate current state of TX (data in in USB terminology) transfers.
            enum class txState_t : uint32_t
            {
                done,
                sending,
                waiting
            };

            /// Reads the data from UART channel on which USB host is located and checks if
            /// received data is internal packet.
            /// param [in,out] cmd  Reference to variable in which read internal command is stored.
            /// returns True if internal command is read, false otherwise.
            bool readInternal(USBLink::internalCMD_t& cmd);

            /// Reads the buffered data already received from UART channel on which USB host is located and checks if
            /// received data is internal packet.
            /// param [in,out] cmd  Reference to variable in which read internal command is stored.
            /// returns True if internal command is read, false otherwise.
            bool checkInternal(USBLink::internalCMD_t& cmd);
        }    // namespace USB

        namespace UART
        {
            enum class dmxState_t : uint8_t
            {
                disabled,
                idle,
                breakChar,
                data,
                waitingTXComplete
            };

            enum class dmxBaudRate_t : uint32_t
            {
                brBreak = 100000,
                brData  = 250000
            };

            namespace ll
            {
                // low-level UART API, MCU specific

                /// Enables the firing of interrupt once the UART data register is empty.
                /// This effectively starts the process of transmitting the data from UART TX buffer to UART interface.
                /// param [in]: channel     UART channel on MCU.
                void enableDataEmptyInt(uint8_t channel);

                /// Performs low-level initialization of the specified UART channel.
                /// param [in]: channel     UART channel on MCU.
                /// param [in]: config_t    Structure containing configuration for given UART channel.
                bool init(uint8_t channel, Board::UART::config_t& config);

                /// Performs low-level deinitialization of the specified UART channel.
                /// param [in]: channel UART channel on MCU.
                bool deInit(uint8_t channel);
            }    // namespace ll

            /// Used to store incoming data from UART to buffer.
            /// param [in]: channel UART channel on MCU.
            /// param [in]: data    Received data.
            void storeIncomingData(uint8_t channel, uint8_t data);

            /// Retrieves the next byte from the outgoing ring buffer.
            /// param [in]: channel             UART channel on MCU.
            /// param [in,out]: data            Reference to variable in which next byte to send is stored.
            /// param [in,out]: remainingBytes  Reference to variable in which total number of bytes remanining in buffer is stored.
            /// returns: True if byte has been successfully retrieved, false otherwise (buffer is empty).
            bool getNextByteToSend(uint8_t channel, uint8_t& data, size_t& remainingBytes);

            /// Checks if there are any bytes to send in outgoing buffer for a given UART channel.
            /// param [in]: channel             UART channel on MCU.
            /// returns: True if there are bytes to send, false otherwise.
            bool bytesToSendAvailable(uint8_t channel);

            /// Used to indicate that the transmission is complete.
            /// param [in]: channel UART channel on MCU.
            void indicateTxComplete(uint8_t channel);

            /// Retrieves pointer to internal DMX buffer array.
            uint8_t* dmxBuffer();
        }    // namespace UART

        namespace io
        {
            /// Continuously reads all digital inputs.
            void checkDigitalInputs();

            /// Removes all readings from digital inputs.
            void flushInputReadings();

            /// Checks if digital outputs need to be updated.
            void checkDigitalOutputs();

            /// Checks if indicator LEDs need to be turned on or off.
            void checkIndicators();

            /// Flashes integrated LEDs on board on startup to indicate that application is about to be loaded.
            void ledFlashStartup();

            /// MCU-specific delay routine used when setting 74HC595 shift register state.
            void sr595wait();

            /// MCU-specific delay routine used when setting 74HC165 shift register state.
            void sr165wait();

            /// Used to temporarily configure all common multiplexer pins as outputs to minimize
            /// the effect of channel-to-channel crosstalk.
            void dischargeMux();

            /// Used to restore pin setup for specified multiplexer.
            void restoreMux(uint8_t muxIndex);

            /// Used as an descriptor for unused pins.
            typedef struct
            {
                core::io::mcuPin_t pin;
                bool               state;
            } unusedIO_t;
        }    // namespace io

        namespace isrHandling
        {
            /// Global ISR handler for all UART events.
            /// param [in]: channel UART channel on MCU.
            void uart(uint8_t channel);

            /// Called in ADC ISR once the conversion is done.
            /// param [in]: adcValue    Retrieved ADC value.
            void adc(uint16_t adcValue);

            // Global ISR handler for timer event on all channels.
            /// param [in]: channel    Channel on timer instance which caused interrupt.
            void timer(uint8_t channel);
        }    // namespace isrHandling

        namespace flash
        {
            typedef struct
            {
                uint32_t address;
                uint32_t size;
            } flashPage_t;

            /// Checks whether the specified flash address is valid / in range for the current MCU.
            bool isInRange(uint32_t address);

            /// Retrieves total flash size in bytes.
            uint32_t size();

            /// Retrieves size of specified flash page.
            uint32_t pageSize(size_t index);

            /// Erases specified flash page.
            bool erasePage(size_t index);

            /// Writes filled page buffer to flash page if needed/supported.
            void writePage(size_t index);

            /// Write 16-bit data to specified address in flash memory.
            bool write16(uint32_t address, uint16_t data);

            /// Write 32-bit data to specified address in flash memory.
            bool write32(uint32_t address, uint32_t data);

            /// Read 8-bit data from specified address in flash memory.
            bool read8(uint32_t address, uint8_t& data);

            /// Read 16-bit data from specified address in flash memory.
            bool read16(uint32_t address, uint16_t& data);

            /// Read 32-bit data from specified address in flash memory.
            bool read32(uint32_t address, uint32_t& data);
        }    // namespace flash
    }        // namespace detail
}    // namespace Board