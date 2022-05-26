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
#include "board/Board.h"
#ifdef USB_OVER_SERIAL
#include "usb-link/Commands.h"
#endif
#include "core/src/MCU.h"

namespace Board::detail
{
    // some boards/SDKs might require periodic calls to certain APIs:
    // enable only if needed
    using updateHook_t = void (*)();

    void registerUpdateHook(updateHook_t hook);

    namespace setup
    {
        void application();
        void bootloader();
    }    // namespace setup

    namespace USB
    {
        /// Used to indicate current state of TX (data in in USB terminology) transfers.
        enum class txState_t : uint32_t
        {
            DONE,
            SENDING,
            WAITING
        };

        void init();

#ifdef USB_OVER_SERIAL
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
#endif

        const void* cfgDescriptor(uint16_t* size);
        const void* deviceDescriptor(uint16_t* size);
        const void* languageString(uint16_t* size);
        const void* manufacturerString(uint16_t* size);
        const void* productString(uint16_t* size);
        const void* serialIDString(uint16_t* size, uint8_t* uid);
    }    // namespace USB

    namespace UART
    {
        enum class dmxState_t : uint8_t
        {
            DISABLED,
            IDLE,
            BREAK_CHAR,
            DATA,
            WAITING_TX_COMPLETE
        };

        enum class dmxBaudRate_t : uint32_t
        {
            BR_BREAK = 100000,
            BR_DATA  = 250000
        };

        struct uartPins_t
        {
            core::mcu::io::pin_t rx;
            core::mcu::io::pin_t tx;
        };

        namespace MCU
        {
            // low-level UART API, MCU specific

            /// Used to start the process of transmitting the data from UART TX buffer to UART interface.
            /// param [in]: channel     UART channel on MCU.
            void startTx(uint8_t channel);

            /// Checks whether the transmission is complete or not for a given UART channel.
            /// param [in]: channel     UART channel on MCU.
            bool isTxComplete(uint8_t channel);

            /// Performs low-level initialization of the specified UART channel.
            /// param [in]: channel     UART channel on MCU.
            /// param [in]: config_t    Structure containing configuration for given UART channel.
            bool init(uint8_t channel, Board::UART::config_t& config);

            /// Performs low-level deinitialization of the specified UART channel.
            /// param [in]: channel UART channel on MCU.
            bool deInit(uint8_t channel);
        }    // namespace MCU

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

        /// Retrieves DMX channel value.
        uint8_t dmxChannelValue(size_t channel);

        /// Global ISR handler for all UART events.
        /// param [in]: channel UART channel on MCU.
        void isr(uint8_t channel);
    }    // namespace UART

    namespace I2C
    {
        struct i2cPins_t
        {
            core::mcu::io::pin_t sda;
            core::mcu::io::pin_t scl;
        };
    }    // namespace I2C

    namespace IO
    {
        void init();

        /// MCU-specific delay routine used to generate correct SPI timings in bit-banging mode.
        void spiWait();

        namespace digitalIn
        {
            // constant used to easily access maximum amount of previous readings for a given digital input
            constexpr inline size_t MAX_READING_COUNT = (8 * sizeof(((Board::IO::digitalIn::readings_t*)0)->readings));

            void init();

            /// Continuously reads all digital inputs.
            void update();

            /// Removes all readings from digital inputs.
            void flush();
        }    // namespace digitalIn

        namespace digitalOut
        {
            constexpr inline size_t NR_OF_RGB_LEDS = NR_OF_DIGITAL_OUTPUTS / 3;

            void init();

            /// Checks if digital outputs need to be updated.
            void update();
        }    // namespace digitalOut

        namespace analog
        {
            namespace MCU
            {
                /// Performs low-level initialization of internal ADC.
                void init();
            }    // namespace MCU

            /// Used to indicate that the new analog reading has been made
            constexpr uint16_t ADC_NEW_READING_FLAG = 0x8000;

            void init();

            /// Called in ADC ISR once the conversion is done.
            /// param [in]: adcValue    Retrieved ADC value.
            void isr(uint16_t adcValue);
        }    // namespace analog

        namespace indicators
        {
            void init();

            /// Checks if indicator LEDs need to be turned on or off.
            void update();

            /// Flashes integrated LEDs on board on startup to indicate that application is about to be loaded.
            void indicateApplicationLoad();

            /// Flashes integrated LEDs on board on startup to indicate that bootloader is about to be loaded.
            void indicateBootloaderLoad();
        }    // namespace indicators

        namespace unused
        {
            /// Used as an descriptor for unused pins.
            struct unusedIO_t
            {
                core::mcu::io::pin_t pin;
                bool                 state;
            };

            void init();
        }    // namespace unused

        namespace bootloader
        {
            void init();
        }    // namespace bootloader
    }        // namespace IO
}    // namespace Board::detail