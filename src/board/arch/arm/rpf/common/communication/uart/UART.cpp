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

#include <map>
#include "board/Board.h"
#include "board/Internal.h"
#include "hardware/uart.h"
#include "hardware/irq.h"
#include "core/src/Timing.h"
#include <Target.h>

namespace
{
    volatile bool _transmitting[CORE_MCU_MAX_UART_INTERFACES];
    uart_inst_t*  _uartInstance[CORE_MCU_MAX_UART_INTERFACES] = {
         uart0,
         uart1
    };

    constexpr uint8_t UART_IRQ[CORE_MCU_MAX_UART_INTERFACES] = {
        UART0_IRQ,
        UART1_IRQ
    };
}    // namespace

namespace Board::detail::UART::MCU
{
    void startTx(uint8_t channel)
    {
        if (!_transmitting[channel])
        {
            uart_set_irq_enables(_uartInstance[channel], true, true);
            core::mcu::isr::uart(channel);
        }
    }

    bool isTxComplete(uint8_t channel)
    {
        return !_transmitting[channel];
    }

    bool deInit(uint8_t channel)
    {
        return true;
    }

    bool init(uint8_t channel, Board::UART::config_t& config)
    {
        uart_init(_uartInstance[channel], config.baudRate);

        gpio_set_function(Board::detail::map::uartPins(channel).tx.index, GPIO_FUNC_UART);
        gpio_set_function(Board::detail::map::uartPins(channel).rx.index, GPIO_FUNC_UART);
        uart_set_hw_flow(_uartInstance[channel], false, false);
        uart_set_format(_uartInstance[channel], 8, config.stopBits, static_cast<uart_parity_t>(config.parity));
        uart_set_fifo_enabled(_uartInstance[channel], false);
        irq_set_enabled(UART_IRQ[channel], true);

        // ignore first rx event (junk)
        core::timing::waitMs(50);

        while (uart_is_readable(_uartInstance[channel]))
        {
            uart_getc(_uartInstance[channel]);
        }

        uart_set_irq_enables(_uartInstance[channel], true, false);

        return true;
    }
}    // namespace Board::detail::UART::MCU

void core::mcu::isr::uart(uint8_t channel)
{
    while (uart_is_readable(_uartInstance[channel]))
    {
        Board::detail::UART::storeIncomingData(channel, uart_getc(_uartInstance[channel]));
    }

    if (uart_is_writable(_uartInstance[channel]))
    {
        size_t  remainingBytes;
        uint8_t value;

        if (Board::detail::UART::getNextByteToSend(channel, value, remainingBytes))
        {
            _transmitting[channel] = true;
            uart_putc_raw(_uartInstance[channel], value);
        }
        else
        {
            // there is no "tx complete" event on rp2040 - if there's nothing more to send, set transmitting to false
            _transmitting[channel] = false;
            uart_set_irq_enables(_uartInstance[channel], true, false);
        }
    }
}

#endif