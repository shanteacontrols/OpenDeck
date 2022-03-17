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

#include "board/Board.h"
#include "board/Internal.h"
#include "core/src/general/Helpers.h"
#include "core/src/general/Timing.h"
#include "nrfx_twim.h"
#include <Target.h>

namespace
{
    nrfx_twim_t _i2cInstance[MAX_I2C_INTERFACES];
}    // namespace

namespace Board::I2C
{
    bool init(uint8_t channel, clockSpeed_t speed)
    {
        if (channel >= MAX_I2C_INTERFACES)
        {
            return false;
        }

        switch (channel)
        {
        case 0:
        {
            _i2cInstance[channel] = NRFX_TWIM_INSTANCE(0);
        }
        break;

        default:
            return false;
        }

        const nrfx_twim_config_t CONFIG = {
            .scl                = NRF_GPIO_PIN_MAP(Board::detail::map::i2cPins(channel).scl.port,
                                    Board::detail::map::i2cPins(channel).scl.index),
            .sda                = NRF_GPIO_PIN_MAP(Board::detail::map::i2cPins(channel).sda.port,
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
            return true;
        }

        return false;
    }

    bool deInit(uint8_t channel)
    {
        if (channel >= MAX_I2C_INTERFACES)
        {
            return false;
        }

        nrfx_twim_uninit(&_i2cInstance[channel]);

        return true;
    }

    bool write(uint8_t channel, uint8_t address, uint8_t* buffer, size_t size)
    {
        if (channel >= MAX_I2C_INTERFACES)
        {
            return false;
        }

        nrfx_twim_xfer_desc_t descriptor = NRFX_TWIM_XFER_DESC_TX(address, buffer, size);

        return nrfx_twim_xfer(&_i2cInstance[channel], &descriptor, 0) == NRFX_SUCCESS;
    }

    bool deviceAvailable(uint8_t channel, uint8_t address)
    {
        if (channel >= MAX_I2C_INTERFACES)
        {
            return false;
        }

        uint8_t               dummy      = 0;
        nrfx_twim_xfer_desc_t descriptor = NRFX_TWIM_XFER_DESC_RX(address, &dummy, 1);

        return nrfx_twim_xfer(&_i2cInstance[channel], &descriptor, 0) == NRFX_SUCCESS;
    }
}    // namespace Board::I2C
