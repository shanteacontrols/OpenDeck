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
#include "core/src/arch/avr/UART.h"
#include "core/src/general/RingBuffer.h"

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

        switch (channel)
        {
        case 0:
            UCSRB_0 |= (1 << UDRIE_0);
            break;

#if UART_INTERFACES > 1
        case 1:
            UCSRB_1 |= (1 << UDRIE_1);
            break;
#endif

        default:
            break;
        }
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

        void reset(uint8_t channel)
        {
            if (channel >= UART_INTERFACES)
                return;

            setLoopbackState(channel, false);

            ATOMIC_SECTION
            {
                switch (channel)
                {
                case 0:
                    UCSRA_0 = 0;
                    UCSRB_0 = 0;
                    UCSRC_0 = 0;
                    UBRR_0 = 0;
                    break;

#if UART_INTERFACES > 1
                case 1:
                    UCSRA_1 = 0;
                    UCSRB_1 = 0;
                    UCSRC_1 = 0;
                    UBRR_1 = 0;
                    break;
#endif

                default:
                    break;
                }
            }

            rxBuffer[channel].reset();
            txBuffer[channel].reset();

            txDone[channel] = true;
        }

        void init(uint32_t baudRate, uint8_t channel)
        {
            if (channel >= UART_INTERFACES)
                return;

            reset(channel);

            int32_t baud_count = ((F_CPU / 8) + (baudRate / 2)) / baudRate;

            if ((baud_count & 1) && baud_count <= 4096)
            {
                switch (channel)
                {
                case 0:
                    UCSRA_0 = (1 << U2X_0);    //double speed uart
                    UBRR_0 = baud_count - 1;
                    break;

#if UART_INTERFACES > 1
                case 1:
                    UCSRA_1 = (1 << U2X_1);    //double speed uart
                    UBRR_1 = baud_count - 1;
                    break;
#endif

                default:
                    break;
                }
            }
            else
            {
                switch (channel)
                {
                case 0:
                    UCSRA_0 = 0;
                    UBRR_0 = (baud_count >> 1) - 1;
                    break;

#if UART_INTERFACES > 1
                case 1:
                    UCSRA_1 = 0;
                    UBRR_1 = (baud_count >> 1) - 1;
                    break;
#endif

                default:
                    break;
                }
            }

            //8 bit, no parity, 1 stop bit
            //enable receiver, transmitter and receive interrupt
            switch (channel)
            {
            case 0:
                UCSRC_0 = (1 << UCSZ1_0) | (1 << UCSZ0_0);
                UCSRB_0 = (1 << RXEN_0) | (1 << TXEN_0) | (1 << RXCIE_0) | (1 << TXCIE_0);
                break;

#if UART_INTERFACES > 1
            case 1:
                UCSRC_1 = (1 << UCSZ1_1) | (1 << UCSZ0_1);
                UCSRB_1 = (1 << RXEN_1) | (1 << TXEN_1) | (1 << RXCIE_1) | (1 << TXCIE_1);
                break;
#endif

            default:
                break;
            }
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
                switch (channel)
                {
                case 0:
                    ATOMIC_SECTION
                    {
                        UDR_0 = data;
                        txDone[0] = false;
                    }

                    return true;
                    break;

#if UART_INTERFACES > 1
                case 1:
                    ATOMIC_SECTION
                    {
                        UDR_1 = data;
                        txDone[1] = false;
                    }

                    return true;
                    break;
#endif

                default:
                    return false;
                }
            }

            while (!txBuffer[channel].insert(data))
                ;
            uartTransmitStart(channel);

            return true;
        }

        bool getLoopbackState(uint8_t channel)
        {
            if (channel >= UART_INTERFACES)
                return false;

            return loopbackEnabled[channel];
        }

        bool isTxEmpty(uint8_t channel)
        {
            if (channel >= UART_INTERFACES)
                return false;

            return txDone[channel];
        }

        uint8_t bytesAvailableRx(uint8_t channel)
        {
            return rxBuffer[channel].count();
        }
    }    // namespace UART
}    // namespace Board

///
/// \brief ISR used to store incoming data from UART to buffer.
/// @{

ISR(USART_RX_vect_0)
{
    uint8_t data = UDR_0;

    if (!loopbackEnabled[0])
    {
        if (rxBuffer[0].insert(data))
        {
#ifdef LED_INDICATORS
            Board::interface::digital::output::indicateMIDItraffic(MIDI::interface_t::din, Board::midiTrafficDirection_t::incoming);
#endif
        }
    }
    else
    {
        if (txBuffer[0].insert(data))
        {
            UCSRB_0 |= (1 << UDRIE_0);
#ifdef LED_INDICATORS
            Board::interface::digital::output::indicateMIDItraffic(MIDI::interface_t::din, Board::midiTrafficDirection_t::outgoing);
            Board::interface::digital::output::indicateMIDItraffic(MIDI::interface_t::din, Board::midiTrafficDirection_t::incoming);
#endif
        }
    }
}

#if UART_INTERFACES > 1
ISR(USART_RX_vect_1)
{
    uint8_t data = UDR_1;

    if (!loopbackEnabled[1])
    {
        if (rxBuffer[1].insert(data))
        {
#ifdef LED_INDICATORS
            Board::interface::digital::output::indicateMIDItraffic(MIDI::interface_t::din, Board::midiTrafficDirection_t::incoming);
#endif
        }
    }
    else
    {
        if (txBuffer[1].insert(data))
        {
#ifdef LED_INDICATORS
            UCSRB_1 |= (1 << UDRIE_1);
            Board::interface::digital::output::indicateMIDItraffic(MIDI::interface_t::din, Board::midiTrafficDirection_t::outgoing);
            Board::interface::digital::output::indicateMIDItraffic(MIDI::interface_t::din, Board::midiTrafficDirection_t::incoming);
#endif
        }
    }
}
#endif

/// @}

///
/// \brief ISR used to write outgoing data in buffer to UART.
/// @{

ISR(USART_UDRE_vect_0)
{
    uint8_t data;

    if (txBuffer[0].remove(data))
    {
        UDR_0 = data;
#ifdef LED_INDICATORS
        Board::interface::digital::output::indicateMIDItraffic(MIDI::interface_t::din, Board::midiTrafficDirection_t::outgoing);
#endif
    }
    else
    {
        //buffer is empty, disable transmit interrupt
        UCSRB_0 &= ~(1 << UDRIE_0);
    }
}

#if UART_INTERFACES > 1
ISR(USART_UDRE_vect_1)
{
    uint8_t data;

    if (txBuffer[1].remove(data))
    {
        UDR_1 = data;
#ifdef LED_INDICATORS
        Board::interface::digital::output::indicateMIDItraffic(MIDI::interface_t::din, Board::midiTrafficDirection_t::outgoing);
#endif
    }
    else
    {
        //buffer is empty, disable transmit interrupt
        UCSRB_1 &= ~(1 << UDRIE_1);
    }
}
#endif

ISR(USART_TX_vect_0)
{
    txDone[0] = true;
}

#if UART_INTERFACES > 1
ISR(USART_TX_vect_1)
{
    txDone[1] = true;
}
#endif

/// @}