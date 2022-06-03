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

#ifdef UART_SUPPORTED

#include "board/Board.h"
#include "board/Internal.h"
#include "core/src/MCU.h"

namespace
{
#ifdef DMX_SUPPORTED
    volatile Board::detail::UART::dmxState_t _dmxState[core::mcu::peripherals::MAX_UART_INTERFACES];
    volatile uint32_t                        _dmxByteCounter;
    uint32_t                                 _dmxBreakBRR;
    uint32_t                                 _dmxDataBRR;
#endif
    volatile bool _transmitting[core::mcu::peripherals::MAX_UART_INTERFACES];
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
#define UDRE_ISR(channel)                                                                  \
    do                                                                                     \
    {                                                                                      \
        uint8_t data;                                                                      \
        size_t  dummy;                                                                     \
        switch (_dmxState[channel])                                                        \
        {                                                                                  \
        case Board::detail::UART::dmxState_t::DISABLED:                                    \
        {                                                                                  \
            if (Board::detail::UART::getNextByteToSend(channel, data, dummy))              \
            {                                                                              \
                UDR(channel) = data;                                                       \
            }                                                                              \
            else                                                                           \
            {                                                                              \
                UCSRB(channel) &= ~(1 << UDRIE(channel));                                  \
            }                                                                              \
        }                                                                                  \
        break;                                                                             \
        case Board::detail::UART::dmxState_t::IDLE:                                        \
        {                                                                                  \
            UCSRB(channel) &= ~(1 << UDRIE(channel));                                      \
            _dmxState[channel] = Board::detail::UART::dmxState_t::BREAK_CHAR;              \
            UDR(channel)       = 0x00;                                                     \
        }                                                                                  \
        break;                                                                             \
        case Board::detail::UART::dmxState_t::DATA:                                        \
        {                                                                                  \
            UDR(channel) = Board::detail::UART::dmxBuffer()->at(_dmxByteCounter++);        \
            if (_dmxByteCounter == 513)                                                    \
            {                                                                              \
                Board::detail::UART::switchDmxBuffer();                                    \
                UCSRB(channel) &= ~(1 << UDRIE(channel));                                  \
                _dmxByteCounter    = 0;                                                    \
                _dmxState[channel] = Board::detail::UART::dmxState_t::WAITING_TX_COMPLETE; \
            }                                                                              \
        }                                                                                  \
        break;                                                                             \
        default:                                                                           \
            break;                                                                         \
        }                                                                                  \
    } while (0)

#define TXC_ISR(channel)                                                \
    do                                                                  \
    {                                                                   \
        switch (_dmxState[channel])                                     \
        {                                                               \
        case Board::detail::UART::dmxState_t::DISABLED:                 \
        {                                                               \
            _transmitting[channel] = false;                             \
        }                                                               \
        break;                                                          \
        case Board::detail::UART::dmxState_t::WAITING_TX_COMPLETE:      \
        {                                                               \
            UCSRB(channel) |= (1 << UDRIE(channel));                    \
            DMX_SET_BREAK_BAUDRATE(channel);                            \
            _dmxState[channel] = Board::detail::UART::dmxState_t::IDLE; \
        }                                                               \
        break;                                                          \
        case Board::detail::UART::dmxState_t::BREAK_CHAR:               \
        {                                                               \
            UCSRB(channel) |= (1 << UDRIE(channel));                    \
            DMX_SET_DATA_BAUDRATE(channel);                             \
            _dmxState[channel] = Board::detail::UART::dmxState_t::DATA; \
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

#define TXC_ISR(channel)                \
    do                                  \
    {                                   \
        _transmitting[channel] = false; \
    } while (0)
#endif

namespace Board::detail::UART::MCU
{
    void startTx(uint8_t channel)
    {
        _transmitting[channel] = true;

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

        default:
            break;
        }
    }

    bool isTxComplete(uint8_t channel)
    {
        return !_transmitting[channel];
    }

    bool deInit(uint8_t channel)
    {
        CORE_MCU_ATOMIC_SECTION
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

            default:
                break;
            }

            _transmitting[channel] = false;
        }

        return true;
    }

    bool init(uint8_t channel, Board::UART::config_t& config)
    {
        if (!deInit(channel))
        {
            return false;
        }

#ifdef DMX_SUPPORTED
        if (config.dmxMode)
        {
            if (config.dmxBuffer == nullptr)
            {
                return false;
            }

            Board::UART::updateDmxBuffer(*config.dmxBuffer);

            config.baudRate = static_cast<uint32_t>(dmxBaudRate_t::BR_BREAK);
        }
#endif

        int32_t baudCount = ((F_CPU / 8) + (config.baudRate / 2)) / config.baudRate;

        if ((baudCount & 1) && baudCount <= 4096)
        {
            switch (channel)
            {
            case 0:
            {
                UCSRA_0 = (1 << U2X_0);    // double speed uart
                UBRR_0  = baudCount - 1;
            }
            break;

#ifdef UCSRA_1
            case 1:
            {
                UCSRA_1 = (1 << U2X_1);    // double speed uart
                UBRR_1  = baudCount - 1;
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
                UBRR_0  = (baudCount >> 1) - 1;
            }
            break;

#ifdef UCSRA_1
            case 1:
            {
                UCSRA_1 = 0;
                UBRR_1  = (baudCount >> 1) - 1;
            }
            break;
#endif

            default:
                break;
            }
        }

#ifdef DMX_SUPPORTED
        _dmxBreakBRR = ((F_CPU / 8) + (static_cast<uint32_t>(dmxBaudRate_t::BR_BREAK) / 2)) / static_cast<uint32_t>(dmxBaudRate_t::BR_BREAK);
        _dmxDataBRR  = ((F_CPU / 8) + (static_cast<uint32_t>(dmxBaudRate_t::BR_DATA) / 2)) / static_cast<uint32_t>(dmxBaudRate_t::BR_DATA);

        _dmxBreakBRR = (_dmxBreakBRR >> 1) - 1;
        _dmxDataBRR  = (_dmxDataBRR >> 1) - 1;
#endif

        // 8 bit data is fixed / non-configurable
        switch (channel)
        {
        case 0:
        {
            UCSRC_0 = (1 << UCSZ1_0) | (1 << UCSZ0_0);

            if (config.type == Board::UART::type_t::RX_TX)
            {
                UCSRB_0 = (1 << RXEN_0) | (1 << TXEN_0) | (1 << RXCIE_0) | (1 << TXCIE_0);
            }
            else if (config.type == Board::UART::type_t::RX)
            {
                UCSRB_0 = (1 << RXEN_0) | (1 << RXCIE_0);
            }
            else if (config.type == Board::UART::type_t::TX)
            {
                UCSRB_0 = (1 << TXEN_0) | (1 << TXCIE_0);
            }

            if (config.stopBits == Board::UART::stopBits_t::TWO)
            {
                UCSRC_0 |= (1 << USBS_0);
            }

            if (config.parity == Board::UART::parity_t::EVEN)
            {
                UCSRC_0 |= (1 << UPM1_0);
            }
            else if (config.parity == Board::UART::parity_t::ODD)
            {
                UCSRC_0 |= (1 << UPM0_0) | (1 << UPM1_0);
            }
        }
        break;

#ifdef UCSRC_1
        case 1:
        {
            UCSRC_1 = (1 << UCSZ1_1) | (1 << UCSZ0_1);

            if (config.type == Board::UART::type_t::RX_TX)
            {
                UCSRB_1 = (1 << RXEN_1) | (1 << TXEN_1) | (1 << RXCIE_1) | (1 << TXCIE_1);
            }
            else if (config.type == Board::UART::type_t::RX)
            {
                UCSRB_1 = (1 << RXEN_1) | (1 << RXCIE_1);
            }
            else if (config.type == Board::UART::type_t::TX)
            {
                UCSRB_1 = (1 << TXEN_1) | (1 << TXCIE_1);
            }

            if (config.stopBits == Board::UART::stopBits_t::TWO)
            {
                UCSRC_1 |= (1 << USBS_1);
            }

            if (config.parity == Board::UART::parity_t::EVEN)
            {
                UCSRC_1 |= (1 << UPM1_1);
            }
            else if (config.parity == Board::UART::parity_t::ODD)
            {
                UCSRC_1 |= (1 << UPM0_1) | (1 << UPM1_1);
            }
        }
        break;
#endif

        default:
            break;
        }

#ifdef DMX_SUPPORTED
        _dmxState[channel] = config.dmxMode ? dmxState_t::IDLE : dmxState_t::DISABLED;
        _dmxByteCounter    = 0;

        if (config.dmxMode)
        {
            startTx(channel);
        }
#endif

        return true;
    }
}    // namespace Board::detail::UART::MCU

/// ISR used to store incoming data from UART to buffer.

ISR(USART_RX_vect_0)
{
    uint8_t data = UDR_0;
    Board::detail::UART::storeIncomingData(0, data);
};

#ifdef UDR_1
ISR(USART_RX_vect_1)
{
    uint8_t data = UDR_1;
    Board::detail::UART::storeIncomingData(1, data);
};
#endif

/// ISR used to write outgoing data in buffer to UART.

ISR(USART_UDRE_vect_0)
{
    UDRE_ISR(0);
};

#ifdef USART_UDRE_vect_1
ISR(USART_UDRE_vect_1)
{
    UDRE_ISR(1);
};
#endif

/// ISR fired once the UART transmission is complete.

ISR(USART_TX_vect_0)
{
    TXC_ISR(0);
};

#ifdef USART_TX_vect_1
ISR(USART_TX_vect_1)
{
    TXC_ISR(1);
};
#endif

#endif
