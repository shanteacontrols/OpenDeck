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

#ifdef USE_UART

#include "board/Board.h"
#include "board/Internal.h"
#include "core/src/general/RingBuffer.h"
#include "core/src/general/Helpers.h"
#include "MCU.h"

#ifndef USB_MIDI_SUPPORTED
#include "board/common/comm/USBOverSerial/USBOverSerial.h"
#include "usb-link/Commands.h"
#endif

//generic UART driver, arch-independent

#ifdef FW_CDC
#undef UART_TX_BUFFER_SIZE
#undef UART_RX_BUFFER_SIZE
#define UART_TX_BUFFER_SIZE CDC_TX_BUFFER_SIZE
#define UART_RX_BUFFER_SIZE CDC_RX_BUFFER_SIZE
#endif

namespace
{
    /// Flag determining whether or not UART loopback functionality is enabled.
    /// When enabled, all incoming UART traffic is immediately passed on to UART TX.
    volatile bool loopbackEnabled[MAX_UART_INTERFACES];

    /// Flag signaling that the transmission is done.
    volatile bool txDone[MAX_UART_INTERFACES];

    /// Flag holding the state of UART interface (whether it's initialized or not).
    bool initialized[MAX_UART_INTERFACES];

    /// Buffer in which outgoing UART data is stored.
    core::RingBuffer<uint8_t, UART_TX_BUFFER_SIZE> txBuffer[MAX_UART_INTERFACES];

    /// Buffer in which incoming UART data is stored.
    core::RingBuffer<uint8_t, UART_RX_BUFFER_SIZE> rxBuffer[MAX_UART_INTERFACES];

#ifndef USB_MIDI_SUPPORTED
    /// Holds the USB state received from USB link MCU
    bool usbConnectionState = false;
#endif

    /// Starts the process of transmitting the data from UART TX buffer to UART interface.
    /// param [in]: channel     UART channel on MCU.
    void uartTransmitStart(uint8_t channel)
    {
        if (channel >= MAX_UART_INTERFACES)
            return;

        txDone[channel] = false;

        Board::detail::UART::ll::enableDataEmptyInt(channel);
    }
}    // namespace

namespace Board
{
    namespace UART
    {
        void setLoopbackState(uint8_t channel, bool state)
        {
            if (channel >= MAX_UART_INTERFACES)
                return;

            loopbackEnabled[channel] = state;
        }

        bool deInit(uint8_t channel)
        {
            if (channel >= MAX_UART_INTERFACES)
                return false;

            if (Board::detail::UART::ll::deInit(channel))
            {
                setLoopbackState(channel, false);

                rxBuffer[channel].reset();
                txBuffer[channel].reset();

                txDone[channel]      = true;
                initialized[channel] = false;

                return true;
            }

            return false;
        }

        initStatus_t init(uint8_t channel, uint32_t baudRate, bool force)
        {
            if (channel >= MAX_UART_INTERFACES)
                return initStatus_t::error;

            if (isInitialized(channel) && !force)
                return initStatus_t::alreadyInit;    //interface already initialized

            if (deInit(channel))
            {
                if (Board::detail::UART::ll::init(channel, baudRate))
                {
                    initialized[channel] = true;
                    return initStatus_t::ok;
                }
            }

            return initStatus_t::error;
        }

        bool isInitialized(uint8_t channel)
        {
            return initialized[channel];
        }

        bool read(uint8_t channel, uint8_t& data)
        {
            data = 0;

            if (channel >= MAX_UART_INTERFACES)
                return false;

            return rxBuffer[channel].remove(data);
        }

        bool write(uint8_t channel, uint8_t data)
        {
            if (channel >= MAX_UART_INTERFACES)
                return false;

            while (!txBuffer[channel].insert(data))
                ;

            uartTransmitStart(channel);

            return true;
        }

        bool isTxEmpty(uint8_t channel)
        {
            if (channel >= MAX_UART_INTERFACES)
                return false;

            return txDone[channel];
        }
    }    // namespace UART

    namespace detail
    {
        namespace UART
        {
            void storeIncomingData(uint8_t channel, uint8_t data)
            {
                if (!loopbackEnabled[channel])
                {
                    rxBuffer[channel].insert(data);
                }
                else
                {
                    if (txBuffer[channel].insert(data))
                    {
                        Board::detail::UART::ll::enableDataEmptyInt(channel);

                        //indicate loopback here since it's run inside interrupt, ie. not visible to the user application
                        Board::io::indicateTraffic(Board::io::dataSource_t::uart, Board::io::dataDirection_t::outgoing);
                        Board::io::indicateTraffic(Board::io::dataSource_t::uart, Board::io::dataDirection_t::incoming);
                    }
                }
            }

            bool getNextByteToSend(uint8_t channel, uint8_t& data, size_t& remainingBytes)
            {
                if (txBuffer[channel].remove(data))
                {
                    remainingBytes = txBuffer[channel].count();
                    return true;
                }
                else
                {
                    remainingBytes = 0;
                    Board::detail::UART::ll::disableDataEmptyInt(channel);
                    return false;
                }
            }

            void indicateTxComplete(uint8_t channel)
            {
                txDone[channel] = true;
            }
        }    // namespace UART
    }        // namespace detail

#ifndef USB_MIDI_SUPPORTED
    namespace USB
    {
        //simulated USB MIDI interface via UART - make this transparent to the application

        uint8_t                             readBuffer[USB_OVER_SERIAL_BUFFER_SIZE];
        Board::USBOverSerial::USBReadPacket readPacket(readBuffer, USB_OVER_SERIAL_BUFFER_SIZE);

        bool isUSBconnected()
        {
            return usbConnectionState;
        }

        bool writeMIDI(MIDI::USBMIDIpacket_t& USBMIDIpacket)
        {
            uint8_t dataArray[4] = {
                USBMIDIpacket.Event,
                USBMIDIpacket.Data1,
                USBMIDIpacket.Data2,
                USBMIDIpacket.Data3
            };

            USBOverSerial::USBWritePacket packet(USBOverSerial::packetType_t::midi, dataArray, 4);

            return USBOverSerial::write(UART_CHANNEL_USB_LINK, packet);
        }

        bool readMIDI(MIDI::USBMIDIpacket_t& USBMIDIpacket)
        {
            bool returnValue = false;

            if (USBOverSerial::read(UART_CHANNEL_USB_LINK, readPacket))
            {
                if (readPacket.type() == USBOverSerial::packetType_t::midi)
                {
                    USBMIDIpacket.Event = readPacket.data()[0];
                    USBMIDIpacket.Data1 = readPacket.data()[1];
                    USBMIDIpacket.Data2 = readPacket.data()[2];
                    USBMIDIpacket.Data3 = readPacket.data()[3];

                    returnValue = true;
                }
                else if (readPacket.type() == USBOverSerial::packetType_t::internal)
                {
                    //internal command
                    if (readPacket.data()[0] == static_cast<uint8_t>(USBLink::internalCMD_t::usbState))
                        usbConnectionState = readPacket.data()[1];
                }

                readPacket.reset();
            }

            return returnValue;
        }
    }    // namespace USB
#endif
}    // namespace Board

#endif