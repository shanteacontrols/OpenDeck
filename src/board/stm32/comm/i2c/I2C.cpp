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

#include "board/Board.h"
#include "board/Internal.h"
#include "core/src/general/Helpers.h"
#include "core/src/general/Timing.h"
#include "MCU.h"

#define I2C_TRANSFER_TIMEOUT_MS 10

namespace
{
    I2C_HandleTypeDef i2cHandler[MAX_I2C_INTERFACES];
}    // namespace

namespace Board
{
    namespace I2C
    {
        bool init(uint8_t channel, clockSpeed_t speed)
        {
            if (channel >= MAX_I2C_INTERFACES)
                return false;

            i2cHandler[channel].Instance             = static_cast<I2C_TypeDef*>(Board::detail::map::i2cDescriptor(channel)->interface());
            i2cHandler[channel].Init.ClockSpeed      = 100000;
            i2cHandler[channel].Init.DutyCycle       = I2C_DUTYCYCLE_2;
            i2cHandler[channel].Init.AddressingMode  = I2C_ADDRESSINGMODE_7BIT;
            i2cHandler[channel].Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
            i2cHandler[channel].Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
            i2cHandler[channel].Init.NoStretchMode   = I2C_NOSTRETCH_DISABLE;

            if (HAL_I2C_Init(&i2cHandler[channel]) != HAL_OK)
            {
                Board::detail::errorHandler();
                return false;
            }

            return true;
        }

        bool deInit(uint8_t channel)
        {
            if (channel >= MAX_I2C_INTERFACES)
                return false;

            return HAL_I2C_DeInit(&i2cHandler[channel]) == HAL_OK;
        }

        bool write(uint8_t channel, uint8_t address, uint8_t* data, size_t size)
        {
            if (channel >= MAX_I2C_INTERFACES)
                return false;

            return HAL_I2C_Master_Transmit(&i2cHandler[channel], address, data, size, I2C_TRANSFER_TIMEOUT_MS) == HAL_OK;
        }
    }    // namespace I2C
}    // namespace Board