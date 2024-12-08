/*

Copyright Igor Petrovic

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

#ifdef PROJECT_TARGET_SUPPORT_UART

#include "board/board.h"
#include "internal.h"
#include <target.h>

#include "core/util/ring_buffer.h"
#include "core/util/util.h"
#include "core/mcu.h"

namespace
{
    core::mcu::uart::Channel<PROJECT_MCU_BUFFER_SIZE_UART_TX, PROJECT_MCU_BUFFER_SIZE_UART_RX> channels[CORE_MCU_MAX_UART_INTERFACES];
}    // namespace

namespace board::uart
{
    initStatus_t init(uint8_t channel, uint32_t baudRate, bool force)
    {
        if (isInitialized(channel))
        {
            if (!force)
            {
                // interface already initialized
                return initStatus_t::ALREADY_INIT;
            }

            if (!deInit(channel))
            {
                return initStatus_t::ERROR;
            }
        }

        core::mcu::uart::Config config(channel,
                                       baudRate,
                                       core::mcu::uart::Config::parity_t::NO,
                                       core::mcu::uart::Config::stopBits_t::ONE,
                                       core::mcu::uart::Config::type_t::RX_TX
#ifdef CORE_MCU_CUSTOM_PERIPHERAL_PINS
                                       ,
                                       board::detail::map::UART_PINS(channel)
#endif
        );

        return channels[channel].init(config) ? initStatus_t::OK : initStatus_t::ERROR;
    }

    bool deInit(uint8_t channel)
    {
        return channels[channel].deInit();
    }

    bool isInitialized(uint8_t channel)
    {
        return channels[channel].isInitialized();
    }

    bool read(uint8_t channel, uint8_t* buffer, size_t& size, const size_t maxSize)
    {
        return channels[channel].read(buffer, size, maxSize);
    }

    bool read(uint8_t channel, uint8_t& value)
    {
        return channels[channel].read(value);
    }

    bool write(uint8_t channel, uint8_t* buffer, size_t size)
    {
        return channels[channel].write(buffer, size);
    }

    bool write(uint8_t channel, uint8_t value)
    {
        return write(channel, &value, 1);
    }

    void setLoopbackState(uint8_t channel, bool state)
    {
        return channels[channel].setLoopbackState(state);
    }
}    // namespace board::uart

#endif