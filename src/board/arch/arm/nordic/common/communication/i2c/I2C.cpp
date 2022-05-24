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

#ifdef I2C_SUPPORTED

#include "board/Board.h"
#include "board/Internal.h"
#include "core/src/util/Util.h"
#include "core/src/Timing.h"
#include "nrfx_twim.h"
#include <Target.h>

namespace
{
    nrfx_twim_t _i2cInstance[core::mcu::peripherals::MAX_I2C_INTERFACES];
    bool        _initialized[core::mcu::peripherals::MAX_I2C_INTERFACES];
}    // namespace

namespace Board::I2C
{
    initStatus_t init(uint8_t channel, clockSpeed_t speed)
    {
        if (channel >= core::mcu::peripherals::MAX_I2C_INTERFACES)
        {
            return initStatus_t::ERROR;
        }

        if (isInitialized(channel))
        {
            return initStatus_t::ALREADY_INIT;
        }

        switch (channel)
        {
        case 0:
        {
            _i2cInstance[channel] = NRFX_TWIM_INSTANCE(0);
        }
        break;

        default:
            return initStatus_t::ERROR;
        }

        const nrfx_twim_config_t CONFIG = {
            .scl                = CORE_NRF_GPIO_PIN_MAP(Board::detail::map::i2cPins(channel).scl.port,
                                         Board::detail::map::i2cPins(channel).scl.index),
            .sda                = CORE_NRF_GPIO_PIN_MAP(Board::detail::map::i2cPins(channel).sda.port,
                                         Board::detail::map::i2cPins(channel).sda.index),
            .frequency          = speed == clockSpeed_t::S100K ? NRF_TWIM_FREQ_100K : NRF_TWIM_FREQ_400K,
            .interrupt_priority = IRQ_PRIORITY_I2C,
            .hold_bus_uninit    = false
        };

        uint32_t context = _i2cInstance[channel].drv_inst_idx;

        if (nrfx_twim_init(&_i2cInstance[channel],
                           &CONFIG,
                           NULL,
                           (void*)context) == NRFX_SUCCESS)
        {
            nrfx_twim_enable(&_i2cInstance[channel]);
            _initialized[channel] = true;

            return initStatus_t::OK;
        }

        return initStatus_t::ERROR;
    }

    bool isInitialized(uint8_t channel)
    {
        return _initialized[channel];
    }

    bool deInit(uint8_t channel)
    {
        if (channel >= core::mcu::peripherals::MAX_I2C_INTERFACES)
        {
            return false;
        }

        nrfx_twim_uninit(&_i2cInstance[channel]);
        _initialized[channel] = false;

        return true;
    }

    bool write(uint8_t channel, uint8_t address, uint8_t* buffer, size_t size)
    {
        if (channel >= core::mcu::peripherals::MAX_I2C_INTERFACES)
        {
            return false;
        }

        nrfx_twim_xfer_desc_t descriptor = NRFX_TWIM_XFER_DESC_TX(address, buffer, size);

        return nrfx_twim_xfer(&_i2cInstance[channel], &descriptor, 0) == NRFX_SUCCESS;
    }

    bool deviceAvailable(uint8_t channel, uint8_t address)
    {
        if (channel >= core::mcu::peripherals::MAX_I2C_INTERFACES)
        {
            return false;
        }

        uint8_t               dummy      = 0;
        nrfx_twim_xfer_desc_t descriptor = NRFX_TWIM_XFER_DESC_RX(address, &dummy, 1);

        return nrfx_twim_xfer(&_i2cInstance[channel], &descriptor, 0) == NRFX_SUCCESS;
    }
}    // namespace Board::I2C

#endif