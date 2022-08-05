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

#ifdef HW_SUPPORT_UART

#include "board/Board.h"
#include "board/Internal.h"
#include "core/src/MCU.h"

namespace
{
    UART_HandleTypeDef                       _uartHandler[CORE_MCU_MAX_UART_INTERFACES];
    volatile Board::detail::UART::dmxState_t _dmxState[CORE_MCU_MAX_UART_INTERFACES];
    volatile uint32_t                        _dmxByteCounter;
    uint32_t                                 _dmxBreakBRR;
    uint32_t                                 _dmxDataBRR;
    volatile bool                            _transmitting[CORE_MCU_MAX_UART_INTERFACES];

    inline void dmxSetBreakBaudrate(uint8_t channel)
    {
        auto instance                              = core::mcu::peripherals::uartDescriptor(channel)->interface();
        static_cast<USART_TypeDef*>(instance)->BRR = _dmxBreakBRR;
    }

    inline void dmxSetDataBaudrate(uint8_t channel)
    {
        auto instance                              = core::mcu::peripherals::uartDescriptor(channel)->interface();
        static_cast<USART_TypeDef*>(instance)->BRR = _dmxDataBRR;
    }
}    // namespace

namespace Board::detail::UART::MCU
{
    void startTx(uint8_t channel)
    {
        if (!_transmitting[channel])
        {
            _transmitting[channel] = true;
            __HAL_UART_ENABLE_IT(&_uartHandler[channel], UART_IT_TXE);
        }
    }

    bool isTxComplete(uint8_t channel)
    {
        return !_transmitting[channel];
    }

    bool deInit(uint8_t channel)
    {
        if (HAL_UART_DeInit(&_uartHandler[channel]) == HAL_OK)
        {
            _transmitting[channel] = false;
            return true;
        }

        return false;
    }

    bool init(uint8_t channel, Board::UART::config_t& config)
    {
        if (!deInit(channel))
        {
            return false;
        }

#ifdef HW_SUPPORT_DMX
        if (config.dmxMode)
        {
            if (config.dmxBuffer == nullptr)
            {
                return false;
            }

            Board::UART::updateDmxBuffer(*config.dmxBuffer);
        }
#endif

        _uartHandler[channel].Instance        = static_cast<USART_TypeDef*>(core::mcu::peripherals::uartDescriptor(channel)->interface());
        _uartHandler[channel].Init.BaudRate   = config.dmxMode ? static_cast<uint32_t>(dmxBaudRate_t::BR_BREAK) : config.baudRate;
        _uartHandler[channel].Init.WordLength = UART_WORDLENGTH_8B;
        _uartHandler[channel].Init.StopBits   = config.stopBits == Board::UART::stopBits_t::ONE ? UART_STOPBITS_1 : UART_STOPBITS_2;

        _dmxState[channel] = config.dmxMode ? dmxState_t::IDLE : dmxState_t::DISABLED;
        _dmxByteCounter    = 0;

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

        _dmxBreakBRR = UART_BRR_SAMPLING16(pclk, static_cast<uint32_t>(dmxBaudRate_t::BR_BREAK));
        _dmxDataBRR  = UART_BRR_SAMPLING16(pclk, static_cast<uint32_t>(dmxBaudRate_t::BR_DATA));

        if (config.parity == Board::UART::parity_t::NO)
        {
            _uartHandler[channel].Init.Parity = UART_PARITY_NONE;
        }
        else if (config.parity == Board::UART::parity_t::EVEN)
        {
            _uartHandler[channel].Init.Parity = UART_PARITY_EVEN;
        }
        else if (config.parity == Board::UART::parity_t::ODD)
        {
            _uartHandler[channel].Init.Parity = UART_PARITY_ODD;
        }

        if (config.type == Board::UART::type_t::RX_TX)
        {
            _uartHandler[channel].Init.Mode = UART_MODE_TX_RX;
        }
        else if (config.type == Board::UART::type_t::RX)
        {
            _uartHandler[channel].Init.Mode = UART_MODE_RX;
        }
        else if (config.type == Board::UART::type_t::TX)
        {
            _uartHandler[channel].Init.Mode = UART_MODE_TX;
        }

        _uartHandler[channel].Init.HwFlowCtl    = UART_HWCONTROL_NONE;
        _uartHandler[channel].Init.OverSampling = UART_OVERSAMPLING_16;

        if (HAL_UART_Init(&_uartHandler[channel]) != HAL_OK)
        {
            return false;
        }

        if ((config.type == Board::UART::type_t::RX_TX) || (config.type == Board::UART::type_t::RX))
        {
            // enable rx interrupt
            __HAL_UART_ENABLE_IT(&_uartHandler[channel], UART_IT_RXNE);
        }

        if (config.dmxMode)
        {
            startTx(channel);
        }

        return true;
    }
}    // namespace Board::detail::UART::MCU

void core::mcu::isr::uart(uint8_t channel)
{
    using namespace Board::detail::UART;

    uint32_t isrflags = _uartHandler[channel].Instance->SR;
    uint32_t cr1its   = _uartHandler[channel].Instance->CR1;
    uint8_t  data     = _uartHandler[channel].Instance->DR;

    bool receiving = (((isrflags & USART_SR_RXNE) != RESET) && ((cr1its & USART_CR1_RXNEIE) != RESET)) ||
                     (((isrflags & USART_SR_ORE) != RESET) && ((cr1its & USART_CR1_RXNEIE) != RESET));

    bool txComplete = ((isrflags & USART_SR_TC) != RESET) && ((cr1its & USART_CR1_TCIE) != RESET);
    bool txEmpty    = ((isrflags & USART_SR_TXE) != RESET) && ((cr1its & USART_CR1_TXEIE) != RESET);

    if (receiving)
    {
        storeIncomingData(channel, data);
    }
    else if (txEmpty || txComplete)
    {
        switch (_dmxState[channel])
        {
        case dmxState_t::DISABLED:
        {
            size_t  remainingBytes;
            uint8_t value;

            if (getNextByteToSend(channel, value, remainingBytes))
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
                    _transmitting[channel] = false;
                }
            }
        }
        break;

        case dmxState_t::WAITING_TX_COMPLETE:
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

                dmxSetBreakBaudrate(channel);
                __HAL_UART_DISABLE_IT(&_uartHandler[channel], UART_IT_TC);
                __HAL_UART_ENABLE_IT(&_uartHandler[channel], UART_IT_TXE);
                _dmxState[channel] = dmxState_t::IDLE;
            }
        }
        break;

        case dmxState_t::IDLE:
        {
            __HAL_UART_DISABLE_IT(&_uartHandler[channel], UART_IT_TXE);
            __HAL_UART_ENABLE_IT(&_uartHandler[channel], UART_IT_TC);

            _dmxState[channel]                 = dmxState_t::BREAK_CHAR;
            _uartHandler[channel].Instance->DR = 0x00;
        }
        break;

        case dmxState_t::BREAK_CHAR:
        {
            if (txComplete)
            {
                dmxSetDataBaudrate(channel);
                __HAL_UART_DISABLE_IT(&_uartHandler[channel], UART_IT_TC);
                __HAL_UART_ENABLE_IT(&_uartHandler[channel], UART_IT_TXE);
                _dmxState[channel] = dmxState_t::DATA;
            }
        }
        break;

        case dmxState_t::DATA:
        {
            _uartHandler[channel].Instance->DR = dmxBuffer()->at(_dmxByteCounter++);

            if (_dmxByteCounter == 513)
            {
                __HAL_UART_ENABLE_IT(&_uartHandler[channel], UART_IT_TC);
                __HAL_UART_DISABLE_IT(&_uartHandler[channel], UART_IT_TXE);
                _dmxByteCounter    = 0;
                _dmxState[channel] = dmxState_t::WAITING_TX_COMPLETE;
                switchDmxBuffer();
            }
        }
        break;

        default:
            break;
        }
    }
}

#endif