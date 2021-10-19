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
#include "core/src/general/Atomic.h"
#include <MCU.h>

namespace
{
    UART_HandleTypeDef                       _uartHandler[MAX_UART_INTERFACES];
    volatile Board::detail::UART::dmxState_t _dmxState[MAX_UART_INTERFACES];
    volatile uint32_t                        _dmxByteCounter;
    uint32_t                                 _dmxBreakBRR;
    uint32_t                                 _dmxDataBRR;
    uint8_t*                                 _dmxBuffer;
}    // namespace

#define DMX_SET_BREAK_BAUDRATE(channel)                                                                        \
    do                                                                                                         \
    {                                                                                                          \
        auto instance                              = Board::detail::map::uartDescriptor(channel)->interface(); \
        static_cast<USART_TypeDef*>(instance)->BRR = _dmxBreakBRR;                                             \
    } while (0)

#define DMX_SET_DATA_BAUDRATE(channel)                                                                         \
    do                                                                                                         \
    {                                                                                                          \
        auto instance                              = Board::detail::map::uartDescriptor(channel)->interface(); \
        static_cast<USART_TypeDef*>(instance)->BRR = _dmxDataBRR;                                              \
    } while (0)

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

                    __HAL_UART_ENABLE_IT(&_uartHandler[channel], UART_IT_TXE);
                }

                bool deInit(uint8_t channel)
                {
                    if (channel >= MAX_UART_INTERFACES)
                        return false;

                    return HAL_UART_DeInit(&_uartHandler[channel]) == HAL_OK;
                }

                bool init(uint8_t channel, Board::UART::config_t& config)
                {
                    if (channel >= MAX_UART_INTERFACES)
                        return false;

                    if (!deInit(channel))
                        return false;

                    _uartHandler[channel].Instance        = static_cast<USART_TypeDef*>(Board::detail::map::uartDescriptor(channel)->interface());
                    _uartHandler[channel].Init.BaudRate   = config.dmxMode ? static_cast<uint32_t>(dmxBaudRate_t::brBreak) : config.baudRate;
                    _uartHandler[channel].Init.WordLength = UART_WORDLENGTH_8B;
                    _uartHandler[channel].Init.StopBits   = config.stopBits == Board::UART::stopBits_t::one ? UART_STOPBITS_1 : UART_STOPBITS_2;

#ifdef DMX_SUPPORTED
                    _dmxState[channel] = config.dmxMode ? dmxState_t::idle : dmxState_t::disabled;
                    _dmxByteCounter    = 0;
                    _dmxBuffer         = Board::detail::UART::dmxBuffer();

                    uint32_t pclk;

#if defined(USART6) && defined(UART9) && defined(UART10)
                    if ((_uartHandler[channel].Instance == USART1) || (_uartHandler[channel].Instance == USART6) || (_uartHandler[channel].Instance == UART9) || (_uartHandler[channel].Instance == UART10))
                    {
                        pclk = HAL_RCC_GetPCLK2Freq();
                    }
#elif defined(USART6)
                    if ((_uartHandler[channel].Instance == USART1) || (_uartHandler[channel].Instance == USART6))
                    {
                        pclk = HAL_RCC_GetPCLK2Freq();
                    }
#else
                    if (_uartHandler[channel].Instance == USART1)
                    {
                        pclk = HAL_RCC_GetPCLK2Freq();
                    }
#endif
                    else
                    {
                        pclk = HAL_RCC_GetPCLK1Freq();
                    }

                    _dmxBreakBRR = UART_BRR_SAMPLING16(pclk, static_cast<uint32_t>(dmxBaudRate_t::brBreak));
                    _dmxDataBRR  = UART_BRR_SAMPLING16(pclk, static_cast<uint32_t>(dmxBaudRate_t::brData));
#endif
                    if (config.parity == Board::UART::parity_t::no)
                        _uartHandler[channel].Init.Parity = UART_PARITY_NONE;
                    else if (config.parity == Board::UART::parity_t::even)
                        _uartHandler[channel].Init.Parity = UART_PARITY_EVEN;
                    else if (config.parity == Board::UART::parity_t::odd)
                        _uartHandler[channel].Init.Parity = UART_PARITY_ODD;

                    if (config.type == Board::UART::type_t::rxTx)
                        _uartHandler[channel].Init.Mode = UART_MODE_TX_RX;
                    else if (config.type == Board::UART::type_t::rx)
                        _uartHandler[channel].Init.Mode = UART_MODE_RX;
                    else if (config.type == Board::UART::type_t::tx)
                        _uartHandler[channel].Init.Mode = UART_MODE_TX;

                    _uartHandler[channel].Init.HwFlowCtl    = UART_HWCONTROL_NONE;
                    _uartHandler[channel].Init.OverSampling = UART_OVERSAMPLING_16;

                    if (HAL_UART_Init(&_uartHandler[channel]) != HAL_OK)
                        return false;

                    if ((config.type == Board::UART::type_t::rxTx) || (config.type == Board::UART::type_t::rx))
                    {
                        // enable rx interrupt
                        __HAL_UART_ENABLE_IT(&_uartHandler[channel], UART_IT_RXNE);
                    }

                    if (config.dmxMode)
                        enableDataEmptyInt(channel);

                    return true;
                }
            }    // namespace ll
        }        // namespace UART

        namespace isrHandling
        {
            void uart(uint8_t channel)
            {
                uint32_t isrflags = _uartHandler[channel].Instance->SR;
                uint32_t cr1its   = _uartHandler[channel].Instance->CR1;
                uint8_t  data     = _uartHandler[channel].Instance->DR;

                bool receiving = (((isrflags & USART_SR_RXNE) != RESET) && ((cr1its & USART_CR1_RXNEIE) != RESET)) ||
                                 (((isrflags & USART_SR_ORE) != RESET) && ((cr1its & USART_CR1_RXNEIE) != RESET));

                bool txComplete = ((isrflags & USART_SR_TC) != RESET) && ((cr1its & USART_CR1_TCIE) != RESET);
                bool txEmpty    = ((isrflags & USART_SR_TXE) != RESET) && ((cr1its & USART_CR1_TXEIE) != RESET);

                if (receiving)
                {
                    Board::detail::UART::storeIncomingData(channel, data);
                }
                else if (txEmpty || txComplete)
                {
                    switch (_dmxState[channel])
                    {
                    case Board::detail::UART::dmxState_t::disabled:
                    {
                        size_t  remainingBytes;
                        uint8_t value;

                        if (Board::detail::UART::getNextByteToSend(channel, value, remainingBytes))
                        {
                            _uartHandler[channel].Instance->DR = value;

                            if (!remainingBytes)
                            {
                                __HAL_UART_ENABLE_IT(&_uartHandler[channel], UART_IT_TC);
                                __HAL_UART_DISABLE_IT(&_uartHandler[channel], UART_IT_TXE);
                            }
                        }
                        else
                        {
                            if (txComplete)
                            {
                                __HAL_UART_DISABLE_IT(&_uartHandler[channel], UART_IT_TC);
                                Board::detail::UART::indicateTxComplete(channel);
                            }
                        }
                    }
                    break;

                    case Board::detail::UART::dmxState_t::waitingTXComplete:
                    {
                        if (txComplete)
                        {
                            // Once TXC interrupt is received while in this state,
                            // change baudrate, enable TX empty isr and exit. The
                            // next time ISR runs, it will check the idle state and
                            // if there is nothing to send, will turn off all TX
                            // interrupts.
                            // Note: switching the baudrate seems to have effect only
                            // the next time ISR runs after switching, which is why a
                            // fall-through to idle state isn't used here. TX empty will
                            // trigger this ISR immediately after exit.

                            DMX_SET_BREAK_BAUDRATE(channel);
                            __HAL_UART_DISABLE_IT(&_uartHandler[channel], UART_IT_TC);
                            __HAL_UART_ENABLE_IT(&_uartHandler[channel], UART_IT_TXE);
                            _dmxState[channel] = Board::detail::UART::dmxState_t::idle;
                        }
                    }
                    break;

                    case Board::detail::UART::dmxState_t::idle:
                    {
                        __HAL_UART_DISABLE_IT(&_uartHandler[channel], UART_IT_TXE);
                        __HAL_UART_ENABLE_IT(&_uartHandler[channel], UART_IT_TC);

                        _dmxState[channel]                 = Board::detail::UART::dmxState_t::breakChar;
                        _uartHandler[channel].Instance->DR = 0x00;
                    }
                    break;

                    case Board::detail::UART::dmxState_t::breakChar:
                    {
                        if (txComplete)
                        {
                            DMX_SET_DATA_BAUDRATE(channel);
                            __HAL_UART_DISABLE_IT(&_uartHandler[channel], UART_IT_TC);
                            __HAL_UART_ENABLE_IT(&_uartHandler[channel], UART_IT_TXE);
                            _dmxState[channel] = Board::detail::UART::dmxState_t::data;
                        }
                    }
                    break;

                    case Board::detail::UART::dmxState_t::data:
                    {
                        _uartHandler[channel].Instance->DR = _dmxBuffer[_dmxByteCounter++];

                        if (_dmxByteCounter == 513)
                        {
                            __HAL_UART_ENABLE_IT(&_uartHandler[channel], UART_IT_TC);
                            __HAL_UART_DISABLE_IT(&_uartHandler[channel], UART_IT_TXE);
                            _dmxByteCounter    = 0;
                            _dmxState[channel] = Board::detail::UART::dmxState_t::waitingTXComplete;
                        }
                    }
                    break;

                    default:
                        break;
                    }
                }
            }
        }    // namespace isrHandling
    }        // namespace detail
}    // namespace Board

#endif