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

#ifdef PROJECT_TARGET_SUPPORT_I2C
#ifndef BOARD_I2C_CUSTOM_IMPL

#include "board/board.h"
#include "internal.h"
#include <target.h>

namespace
{
    bool initialized[CORE_MCU_MAX_I2C_INTERFACES];
}    // namespace

namespace board::i2c
{
    initStatus_t init(uint8_t channel, clockSpeed_t speed)
    {
        if (isInitialized(channel))
        {
            return initStatus_t::ALREADY_INIT;
        }

        core::mcu::i2c::Config config(static_cast<core::mcu::i2c::Config::clockSpeed_t>(speed)
#ifdef CORE_MCU_CUSTOM_PERIPHERAL_PINS
                                          ,
                                      board::detail::map::I2C_PINS(channel)
#endif
        );

        if (core::mcu::i2c::init(channel, config))
        {
            initialized[channel] = true;
            return initStatus_t::OK;
        }

        return initStatus_t::ERROR;
    }

    bool isInitialized(uint8_t channel)
    {
        if (channel >= CORE_MCU_MAX_I2C_INTERFACES)
        {
            return false;
        }

        return initialized[channel];
    }

    bool deInit(uint8_t channel)
    {
        if (core::mcu::i2c::deInit(channel))
        {
            initialized[channel] = false;
            return true;
        }

        return false;
    }

    bool write(uint8_t channel, uint8_t address, uint8_t* buffer, size_t size)
    {
        return core::mcu::i2c::write(channel, address, buffer, size);
    }

    bool deviceAvailable(uint8_t channel, uint8_t address)
    {
        return core::mcu::i2c::deviceAvailable(channel, address);
    }
}    // namespace board::i2c

#endif
#endif