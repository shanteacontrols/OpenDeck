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

                    //enable transmission done interrupt
                    __HAL_UART_ENABLE_IT(&uartHandler[channel], UART_IT_TC);

                    //enable data not empty interrupt
                    __HAL_UART_ENABLE_IT(&uartHandler[channel], UART_IT_RXNE);

                    return true;
                }

                void directWrite(uint8_t channel, uint8_t data)
                {
                    uartHandler[channel].Instance->DR = data;
                }
            }    // namespace ll
        }        // namespace UART

        namespace isrHandling
        {
            void uart(uint8_t channel)
            {
                uint32_t isrflags     = READ_REG(uartHandler[channel].Instance->SR);
                uint8_t  data         = uartHandler[channel].Instance->DR;
                uint32_t cr1its       = READ_REG(uartHandler[channel].Instance->CR1);
                uint32_t errorflags   = (isrflags & (uint32_t)(USART_SR_PE | USART_SR_FE | USART_SR_ORE | USART_SR_NE));
                bool     verifyTxDone = true;

                if (errorflags == RESET)
                {
                    //transmitting
                    if (((isrflags & USART_SR_TXE) != RESET) && ((cr1its & USART_CR1_TXEIE) != RESET))
                    {
                        if (Board::detail::UART::getNextByteToSend(channel, data))
                        {
                            uartHandler[channel].Instance->DR = data;

                            //more data to send - don't verify tx done interrupt yet
                            verifyTxDone = false;
                        }
                    }
                    else
                    {
                        //receiving
                        if (((isrflags & USART_SR_RXNE) != RESET) && ((cr1its & USART_CR1_RXNEIE) != RESET))
                            Board::detail::UART::storeIncomingData(channel, data);
                    }

                    if (verifyTxDone)
                    {
                        //verify if transmission done interrupt has occured
                        if (__HAL_UART_GET_IT_SOURCE(&uartHandler[channel], UART_IT_TC) != RESET)
                        {
                            Board::detail::UART::indicateTxComplete(channel);
                            __HAL_UART_CLEAR_FLAG(&uartHandler[channel], UART_IT_TC);
                        }
                    }
                }
            }
        }    // namespace isrHandling
    }        // namespace detail
}    // namespace Board

#endif