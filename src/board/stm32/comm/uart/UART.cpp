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
#include "MCU.h"

namespace
{
    UART_HandleTypeDef uartHandler[MAX_UART_INTERFACES];
}

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

                    __HAL_UART_ENABLE_IT(&uartHandler[channel], UART_IT_TXE);
                }

                void disableDataEmptyInt(uint8_t channel)
                {
                    if (channel >= MAX_UART_INTERFACES)
                        return;

                    __HAL_UART_DISABLE_IT(&uartHandler[channel], UART_IT_TXE);
                }

                bool deInit(uint8_t channel)
                {
                    if (channel >= MAX_UART_INTERFACES)
                        return false;

                    return HAL_UART_DeInit(&uartHandler[channel]) == HAL_OK;
                }

                bool init(uint8_t channel, uint32_t baudRate)
                {
                    if (channel >= MAX_UART_INTERFACES)
                        return false;

                    if (!deInit(channel))
                        return false;

                    uartHandler[channel].Instance          = static_cast<USART_TypeDef*>(Board::detail::map::uartDescriptor(channel)->interface());
                    uartHandler[channel].Init.BaudRate     = baudRate;
                    uartHandler[channel].Init.WordLength   = UART_WORDLENGTH_8B;
                    uartHandler[channel].Init.StopBits     = UART_STOPBITS_1;
                    uartHandler[channel].Init.Parity       = UART_PARITY_NONE;
                    uartHandler[channel].Init.Mode         = UART_MODE_TX_RX;
                    uartHandler[channel].Init.HwFlowCtl    = UART_HWCONTROL_NONE;
                    uartHandler[channel].Init.OverSampling = UART_OVERSAMPLING_16;

                    if (HAL_UART_Init(&uartHandler[channel]) != HAL_OK)
                        return false;

                    //enable rx interrupt
                    __HAL_UART_ENABLE_IT(&uartHandler[channel], UART_IT_RXNE);

                    return true;
                }
            }    // namespace ll
        }        // namespace UART

        namespace isrHandling
        {
            void uart(uint8_t channel)
            {
                uint32_t isrflags = uartHandler[channel].Instance->SR;
                uint32_t cr1its   = uartHandler[channel].Instance->CR1;
                uint8_t  data     = uartHandler[channel].Instance->DR;

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
                    size_t remainingBytes;

                    if (Board::detail::UART::getNextByteToSend(channel, data, remainingBytes))
                    {
                        if (!remainingBytes)
                        {
                            __HAL_UART_ENABLE_IT(&uartHandler[channel], UART_IT_TC);
                            __HAL_UART_DISABLE_IT(&uartHandler[channel], UART_IT_TXE);
                        }

                        uartHandler[channel].Instance->DR = data;
                    }
                    else
                    {
                        if (txComplete)
                        {
                            __HAL_UART_DISABLE_IT(&uartHandler[channel], UART_IT_TC);
                            Board::detail::UART::indicateTxComplete(channel);
                        }
                    }
                }
            }
        }    // namespace isrHandling
    }        // namespace detail
}    // namespace Board

#endif