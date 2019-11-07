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

#include "board/Board.h"
#include "board/Internal.h"
#include "core/src/general/RingBuffer.h"

//generic UART driver, arch-independent

#define TX_BUFFER_SIZE 50
#define RX_BUFFER_SIZE 50

namespace
{
    ///
    /// \brief Flag determining whether or not UART loopback functionality is enabled.
    /// When enabled, all incoming UART traffic is immediately passed on to UART TX.
    ///
    bool loopbackEnabled[UART_INTERFACES];

    ///
    /// \brief Flag signaling that the transmission is done.
    ///
    bool txDone[UART_INTERFACES];

    ///
    /// \brief Buffer in which outgoing UART data is stored.
    ///
    core::RingBuffer<uint8_t, TX_BUFFER_SIZE> txBuffer[UART_INTERFACES];

    ///
    /// \brief Buffer in which incoming UART data is stored.
    ///
    core::RingBuffer<uint8_t, RX_BUFFER_SIZE> rxBuffer[UART_INTERFACES];

    ///
    /// \brief Starts the process of transmitting the data from UART TX buffer to UART interface.
    /// @param [in] channel     UART channel on MCU.
    ///
    void uartTransmitStart(uint8_t channel)
    {
        if (channel >= UART_INTERFACES)
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
            if (channel >= UART_INTERFACES)
                return;

            loopbackEnabled[channel] = state;
        }

        void deInit(uint8_t channel)
        {
            if (channel >= UART_INTERFACES)
                return;

            setLoopbackState(channel, false);

            Board::detail::UART::ll::deInit(channel);

            rxBuffer[channel].reset();
            txBuffer[channel].reset();

            txDone[channel] = true;
        }

        void init(uint8_t channel, uint32_t baudRate)
        {
            if (channel >= UART_INTERFACES)
                return;

            deInit(channel);
            Board::detail::UART::ll::init(channel, baudRate);
        }

        bool read(uint8_t channel, uint8_t& data)
        {
            data = 0;

            if (channel >= UART_INTERFACES)
                return false;

            return rxBuffer[channel].remove(data);
        }

        bool write(uint8_t channel, uint8_t data)
        {
            if (channel >= UART_INTERFACES)
                return false;

            //if:
            // *outgoing buffer is empty
            // * UART data register is empty
            // txDone is marked as true
            //write the byte to the data register directly
            if (txDone[channel] && txBuffer[channel].isEmpty())
            {
                txDone[channel] = false;
                Board::detail::UART::ll::directWrite(channel, data);
                return true;
            }

            while (!txBuffer[channel].insert(data))
                ;

            uartTransmitStart(channel);

            return true;
        }

        bool isTxEmpty(uint8_t channel)
        {
            if (channel >= UART_INTERFACES)
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
                    if (rxBuffer[channel].insert(data))
                    {
#ifdef FW_APP
#ifdef LED_INDICATORS
                        Board::detail::io::indicateMIDItraffic(MIDI::interface_t::din, Board::detail::midiTrafficDirection_t::incoming);
#endif
#endif
                    }
                }
                else
                {
                    if (txBuffer[channel].insert(data))
                    {
                        Board::detail::UART::ll::enableDataEmptyInt(channel);
#ifdef FW_APP
#ifdef LED_INDICATORS
                        Board::detail::io::indicateMIDItraffic(MIDI::interface_t::din, Board::detail::midiTrafficDirection_t::outgoing);
                        Board::detail::io::indicateMIDItraffic(MIDI::interface_t::din, Board::detail::midiTrafficDirection_t::incoming);
#endif
#endif
                    }
                }
            }

            bool getNextByteToSend(uint8_t channel, uint8_t& data)
            {
                if (txBuffer[channel].remove(data))
                {
#ifdef FW_APP
#ifdef LED_INDICATORS
                    Board::detail::io::indicateMIDItraffic(MIDI::interface_t::din, Board::detail::midiTrafficDirection_t::outgoing);
#endif
#endif
                    return true;
                }
                else
                {
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
}    // namespace Board