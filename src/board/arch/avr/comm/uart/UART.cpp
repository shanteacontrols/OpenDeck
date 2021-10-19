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
#include "core/src/arch/avr/Atomic.h"
#include "core/src/arch/avr/UART.h"
#include <MCU.h>

namespace
{
#ifdef DMX_SUPPORTED
    volatile Board::detail::UART::dmxState_t _dmxState[MAX_UART_INTERFACES];
    volatile uint32_t                        _dmxByteCounter;
    uint32_t                                 _dmxBreakBRR;
    uint32_t                                 _dmxDataBRR;
    uint8_t*                                 _dmxBuffer;
#endif
}    // namespace

// these macros are used to avoid function calls in ISR as much as possible and to avoid
// code repetition with only the register names being different

#define _UBRR_GEN(x)  UBRR_##x
#define UBRR(x)       _UBRR_GEN(x)
#define _UDR_GEN(x)   UDR_##x
#define UDR(x)        _UDR_GEN(x)
#define _UCSRB_GEN(x) UCSRB_##x
#define UCSRB(x)      _UCSRB_GEN(x)
#define _UDRIE_GEN(x) UDRIE_##x
#define UDRIE(x)      _UDRIE_GEN(x)

#ifdef DMX_SUPPORTED
#define DMX_SET_BREAK_BAUDRATE(channel) (UBRR(channel) = _dmxBreakBRR)
#define DMX_SET_DATA_BAUDRATE(channel)  (UBRR(channel) = _dmxDataBRR)
#endif

#ifdef DMX_SUPPORTED
#define UDRE_ISR(channel)                                                                \
    do                                                                                   \
    {                                                                                    \
        uint8_t data;                                                                    \
        size_t  dummy;                                                                   \
        switch (_dmxState[channel])                                                      \
        {                                                                                \
        case Board::detail::UART::dmxState_t::disabled:                                  \
        {                                                                                \
            if (Board::detail::UART::getNextByteToSend(channel, data, dummy))            \
            {                                                                            \
                UDR(channel) = data;                                                     \
            }                                                                            \
            else                                                                         \
            {                                                                            \
                UCSRB(channel) &= ~(1 << UDRIE(channel));                                \
            }                                                                            \
        }                                                                                \
        break;                                                                           \
        case Board::detail::UART::dmxState_t::idle:                                      \
        {                                                                                \
            UCSRB(channel) &= ~(1 << UDRIE(channel));                                    \
            _dmxState[channel] = Board::detail::UART::dmxState_t::breakChar;             \
            UDR(channel)       = 0x00;                                                   \
        }                                                                                \
        break;                                                                           \
        case Board::detail::UART::dmxState_t::data:                                      \
        {                                                                                \
            UDR(channel) = _dmxBuffer[_dmxByteCounter++];                                \
            if (_dmxByteCounter == 513)                                                  \
            {                                                                            \
                UCSRB(channel) &= ~(1 << UDRIE(channel));                                \
                _dmxByteCounter    = 0;                                                  \
                _dmxState[channel] = Board::detail::UART::dmxState_t::waitingTXComplete; \
            }                                                                            \
        }                                                                                \
        break;                                                                           \
        default:                                                                         \
            break;                                                                       \
        }                                                                                \
    } while (0)

#define TXC_ISR(channel)                                                \
    do                                                                  \
    {                                                                   \
        switch (_dmxState[channel])                                     \
        {                                                               \
        case Board::detail::UART::dmxState_t::disabled:                 \
        {                                                               \
            Board::detail::UART::indicateTxComplete(channel);           \
        }                                                               \
        break;                                                          \
        case Board::detail::UART::dmxState_t::waitingTXComplete:        \
        {                                                               \
            UCSRB(channel) |= (1 << UDRIE(channel));                    \
            DMX_SET_BREAK_BAUDRATE(channel);                            \
            _dmxState[channel] = Board::detail::UART::dmxState_t::idle; \
        }                                                               \
        break;                                                          \
        case Board::detail::UART::dmxState_t::breakChar:                \
        {                                                               \
            UCSRB(channel) |= (1 << UDRIE(channel));                    \
            DMX_SET_DATA_BAUDRATE(channel);                             \
            _dmxState[channel] = Board::detail::UART::dmxState_t::data; \
        }                                                               \
        break;                                                          \
        default:                                                        \
            break;                                                      \
        }                                                               \
    } while (0)
#else
#define UDRE_ISR(channel)                                                 \
    do                                                                    \
    {                                                                     \
        uint8_t data;                                                     \
        size_t  dummy;                                                    \
        if (Board::detail::UART::getNextByteToSend(channel, data, dummy)) \
        {                                                                 \
            UDR(channel) = data;                                          \
        }                                                                 \
        else                                                              \
        {                                                                 \
            UCSRB(channel) &= ~(1 << UDRIE(channel));                     \
        }                                                                 \
    } while (0)

#define TXC_ISR(channel)                                  \
    do                                                    \
    {                                                     \
        Board::detail::UART::indicateTxComplete(channel); \
    } while (0)
#endif

namespace Board
{
    namespace detail
    {
        namespace UART
        {
            namespace ll
            {
                void enableDataEmptyInt(uint8_t channel)
                {
                    if (channel >= MAX_UART_INTERFACES)
                        return;

                    switch (channel)
                    {
                    case 0:
                    {
                        UCSRB_0 |= (1 << UDRIE_0);
                    }
                    break;

#ifdef UCSRB_1
                    case 1:
                    {
                        UCSRB_1 |= (1 << UDRIE_1);
                    }
                    break;
#endif

#ifdef UCSRB_2
                    case 2:
                    {
                        UCSRB_2 |= (1 << UDRIE_2);
                    }
                    break;
#endif

                    default:
                        break;
                    }
                }

                bool deInit(uint8_t channel)
                {
                    if (channel >= MAX_UART_INTERFACES)
                        return false;

                    ATOMIC_SECTION
                    {
                        switch (channel)
                        {
                        case 0:
                        {
                            UCSRA_0 = 0;
                            UCSRB_0 = 0;
                            UCSRC_0 = 0;
                            UBRR_0  = 0;
                        }
                        break;

#ifdef UCSRB_1
                        case 1:
                        {
                            UCSRA_1 = 0;
                            UCSRB_1 = 0;
                            UCSRC_1 = 0;
                            UBRR_1  = 0;
                        }
                        break;
#endif

#ifdef UCSRB_2
                        case 2:
                        {
                            UCSRA_2 = 0;
                            UCSRB_2 = 0;
                            UCSRC_2 = 0;
                            UBRR_2  = 0;
                        }
                        break;
#endif

                        default:
                            break;
                        }
                    }

                    return true;
                }

                bool init(uint8_t channel, Board::UART::config_t& config)
                {
                    if (channel >= MAX_UART_INTERFACES)
                        return false;

                    if (!deInit(channel))
                        return false;

#ifdef DMX_SUPPORTED
                    if (config.dmxMode)
                        config.baudRate = static_cast<uint32_t>(dmxBaudRate_t::brBreak);
#endif

                    int32_t baud_count = ((F_CPU / 8) + (config.baudRate / 2)) / config.baudRate;

                    if ((baud_count & 1) && baud_count <= 4096)
                    {
                        switch (channel)
                        {
                        case 0:
                        {
                            UCSRA_0 = (1 << U2X_0);    // double speed uart
                            UBRR_0  = baud_count - 1;
                        }
                        break;

#ifdef UCSRA_1
                        case 1:
                        {
                            UCSRA_1 = (1 << U2X_1);    // double speed uart
                            UBRR_1  = baud_count - 1;
                        }
                        break;
#endif

#ifdef UCSRA_2
                        case 2:
                        {
                            UCSRA_2 = (1 << U2X_2);    // double speed uart
                            UBRR_2  = baud_count - 1;
                        }
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
                        {
                            UCSRA_0 = 0;
                            UBRR_0  = (baud_count >> 1) - 1;
                        }
                        break;

#ifdef UCSRA_1
                        case 1:
                        {
                            UCSRA_1 = 0;
                            UBRR_1  = (baud_count >> 1) - 1;
                        }
                        break;
#endif

#ifdef UCSRA_2
                        case 2:
                        {
                            UCSRA_2 = 0;
                            UBRR_2  = (baud_count >> 1) - 1;
                        }
                        break;
#endif

                        default:
                            break;
                        }
                    }

#ifdef DMX_SUPPORTED
                    _dmxBreakBRR = ((F_CPU / 8) + (static_cast<uint32_t>(dmxBaudRate_t::brBreak) / 2)) / static_cast<uint32_t>(dmxBaudRate_t::brBreak);
                    _dmxDataBRR  = ((F_CPU / 8) + (static_cast<uint32_t>(dmxBaudRate_t::brData) / 2)) / static_cast<uint32_t>(dmxBaudRate_t::brData);

                    _dmxBreakBRR = (_dmxBreakBRR >> 1) - 1;
                    _dmxDataBRR  = (_dmxDataBRR >> 1) - 1;
#endif

                    // 8 bit data is fixed / non-configurable
                    switch (channel)
                    {
                    case 0:
                    {
                        UCSRC_0 = (1 << UCSZ1_0) | (1 << UCSZ0_0);

                        if (config.type == Board::UART::type_t::rxTx)
                            UCSRB_0 = (1 << RXEN_0) | (1 << TXEN_0) | (1 << RXCIE_0) | (1 << TXCIE_0);
                        else if (config.type == Board::UART::type_t::rx)
                            UCSRB_0 = (1 << RXEN_0) | (1 << RXCIE_0);
                        else if (config.type == Board::UART::type_t::tx)
                            UCSRB_0 = (1 << TXEN_0) | (1 << TXCIE_0);

                        if (config.stopBits == Board::UART::stopBits_t::two)
                            UCSRC_0 |= (1 << USBS_0);

                        if (config.parity == Board::UART::parity_t::even)
                            UCSRC_0 |= (1 << UPM1_0);
                        else if (config.parity == Board::UART::parity_t::odd)
                            UCSRC_0 |= (1 << UPM0_0) | (1 << UPM1_0);
                    }
                    break;

#ifdef UCSRC_1
                    case 1:
                    {
                        UCSRC_1 = (1 << UCSZ1_1) | (1 << UCSZ0_1);

                        if (config.type == Board::UART::type_t::rxTx)
                            UCSRB_1 = (1 << RXEN_1) | (1 << TXEN_1) | (1 << RXCIE_1) | (1 << TXCIE_1);
                        else if (config.type == Board::UART::type_t::rx)
                            UCSRB_1 = (1 << RXEN_1) | (1 << RXCIE_1);
                        else if (config.type == Board::UART::type_t::tx)
                            UCSRB_1 = (1 << TXEN_1) | (1 << TXCIE_1);

                        if (config.stopBits == Board::UART::stopBits_t::two)
                            UCSRC_1 |= (1 << USBS_1);

                        if (config.parity == Board::UART::parity_t::even)
                            UCSRC_1 |= (1 << UPM1_1);
                        else if (config.parity == Board::UART::parity_t::odd)
                            UCSRC_1 |= (1 << UPM0_1) | (1 << UPM1_1);
                    }
                    break;
#endif

#ifdef UCSRC_2
                    case 2:
                    {
                        UCSRC_2 = (1 << UCSZ1_2) | (1 << UCSZ0_2);

                        if (config.type == Board::UART::type_t::rxTx)
                            UCSRB_2 = (1 << RXEN_2) | (1 << TXEN_2) | (1 << RXCIE_2) | (1 << TXCIE_2);
                        else if (config.type == Board::UART::type_t::rx)
                            UCSRB_2 = (1 << RXEN_2) | (1 << RXCIE_2);
                        else if (config.type == Board::UART::type_t::tx)
                            UCSRB_2 = (1 << TXEN_2) | (1 << TXCIE_2);

                        if (config.stopBits == Board::UART::stopBits_t::two)
                            UCSRC_2 |= (1 << USBS_2);

                        if (config.parity == Board::UART::parity_t::even)
                            UCSRC_2 |= (1 << UPM1_2);
                        else if (config.parity == Board::UART::parity_t::odd)
                            UCSRC_2 |= (1 << UPM0_2) | (1 << UPM1_2);
                    }
                    break;
#endif

                    default:
                        break;
                    }

#ifdef DMX_SUPPORTED
                    _dmxState[channel] = config.dmxMode ? dmxState_t::idle : dmxState_t::disabled;
                    _dmxByteCounter    = 0;
                    _dmxBuffer         = Board::detail::UART::dmxBuffer();

                    if (config.dmxMode)
                        enableDataEmptyInt(channel);
#endif

                    return true;
                }
            }    // namespace ll
        }        // namespace UART
    }            // namespace detail
}    // namespace Board

/// ISR used to store incoming data from UART to buffer.

ISR(USART_RX_vect_0)
{
    uint8_t data = UDR_0;
    Board::detail::UART::storeIncomingData(0, data);
}

#ifdef UDR_1
ISR(USART_RX_vect_1)
{
    uint8_t data = UDR_1;
    Board::detail::UART::storeIncomingData(1, data);
}
#endif

#ifdef UDR_2
ISR(USART_RX_vect_2)
{
    uint8_t data = UDR_2;
    Board::detail::UART::storeIncomingData(2, data);
}
#endif

///

/// ISR used to write outgoing data in buffer to UART.

ISR(USART_UDRE_vect_0)
{
    UDRE_ISR(0);
}

#ifdef UDR_1
ISR(USART_UDRE_vect_1)
{
    UDRE_ISR(1);
}
#endif

#ifdef UDR_2
ISR(USART_UDRE_vect_2)
{
    UDRE_ISR(2);
}
#endif

///

/// ISR fired once the UART transmission is complete.

ISR(USART_TX_vect_0)
{
    TXC_ISR(0);
}

#ifdef UDR_1
ISR(USART_TX_vect_1)
{
    TXC_ISR(1);
}
#endif

#ifdef UDR_2
ISR(USART_TX_vect_2)
{
    TXC_ISR(2);
}
#endif

///

#endif
