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

#include <util/twi.h>
#include "board/Board.h"
#include "core/src/general/Helpers.h"
#include "core/src/general/Timing.h"
#include <MCU.h>

#define I2C_TRANSFER_TIMEOUT_MS 10

// note: on AVR, only 1 I2C channel is supported with the index 0

namespace
{
    uint32_t _currentTime;

#define TIMEOUT_CHECK(register, bit)                                                         \
    do                                                                                       \
    {                                                                                        \
        _currentTime = core::timing::currentRunTimeMs();                                     \
        while (!BIT_READ(register, bit))                                                     \
        {                                                                                    \
            if ((core::timing::currentRunTimeMs() - _currentTime) > I2C_TRANSFER_TIMEOUT_MS) \
            {                                                                                \
                TWCR = 0;                                                                    \
                TWDR = 0;                                                                    \
                return false;                                                                \
            }                                                                                \
        }                                                                                    \
    } while (0)

    inline bool startTransfer(uint8_t address)
    {
        // enable interrupt flag
        // enable start bit (set to master)
        TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);

        // wait for interrupt flag to be cleared
        TIMEOUT_CHECK(TWCR, TWINT);

        // check the value of TWI status register
        uint8_t status = TW_STATUS & 0xF8;

        if ((status != TW_START) && (status != TW_REP_START))
            return false;

        // send device address
        TWDR = address << 1;
        TWCR = (1 << TWINT) | (1 << TWEN);

        TIMEOUT_CHECK(TWCR, TWINT);

        status = TW_STATUS & 0xF8;

        if ((status != TW_MT_SLA_ACK) && (status != TW_MR_SLA_ACK))
            return false;

        return true;
    }

    inline bool endTransfer()
    {
        // wait until all ongoing transmissions are stopped
        TWCR |= (1 << TWSTO);

        TIMEOUT_CHECK(TWCR, TWSTO);

        return true;
    }
}    // namespace

namespace Board
{
    namespace I2C
    {
        bool init(uint8_t channel, clockSpeed_t speed)
        {
            if (channel >= MAX_I2C_INTERFACES)
                return false;

            // no prescaling
            TWSR = 0x00;

            // use formula as per datasheet
            TWBR = ((F_CPU / static_cast<uint32_t>(speed)) - 16) / 2;

            return true;
        }

        bool deInit(uint8_t channel)
        {
            if (channel >= MAX_I2C_INTERFACES)
                return false;

            TWCR = 0;
            return true;
        }

        bool write(uint8_t channel, uint8_t address, uint8_t* buffer, size_t size)
        {
            if (channel >= MAX_I2C_INTERFACES)
                return false;

            if (!startTransfer(address))
                return false;

            for (size_t i = 0; i < size; i++)
            {
                TWDR = buffer[i];
                TWCR = (1 << TWINT) | (1 << TWEN);

                // wait for interrupt flag to be cleared
                TIMEOUT_CHECK(TWCR, TWINT);

                if ((TW_STATUS & 0xF8) != TW_MT_DATA_ACK)
                    return false;
            }

            return endTransfer();
        }

        bool deviceAvailable(uint8_t channel, uint8_t address)
        {
            if (channel >= MAX_I2C_INTERFACES)
                return false;

            bool found = false;

            if (startTransfer(address))
            {
                found = true;
            }

            if (!endTransfer())
                return false;

            return found;
        }
    }    // namespace I2C
}    // namespace Board