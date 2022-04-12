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
#include "core/src/util/Util.h"
#include "core/src/Timing.h"
#include "core/src/util/RingBuffer.h"
#include "core/src/MCU.h"

// note: on AVR, only 1 I2C channel is supported with the index 0

namespace
{
    constexpr uint32_t                                  I2C_TRANSFER_TIMEOUT_MS = 10;
    constexpr uint8_t                                   TWCR_CLR_MASK           = 0x0F;
    uint32_t                                            _currentTime;
    core::util::RingBuffer<uint8_t, I2C_TX_BUFFER_SIZE> _txBuffer;
    uint8_t                                             _address;
    volatile bool                                       _txBusy;

#define TIMEOUT_CHECK(register, bit)                                           \
    do                                                                         \
    {                                                                          \
        _currentTime = core::timing::ms();                                     \
        while (!core::util::BIT_READ(register, bit))                           \
        {                                                                      \
            if ((core::timing::ms() - _currentTime) > I2C_TRANSFER_TIMEOUT_MS) \
            {                                                                  \
                TWCR = 0;                                                      \
                TWDR = 0;                                                      \
                return false;                                                  \
            }                                                                  \
        }                                                                      \
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
        {
            return false;
        }

        // send device address
        TWDR = address << 1;
        TWCR = (1 << TWINT) | (1 << TWEN);

        TIMEOUT_CHECK(TWCR, TWINT);

        status = TW_STATUS & 0xF8;

        if ((status != TW_MT_SLA_ACK) && (status != TW_MR_SLA_ACK))
        {
            return false;
        }

        return true;
    }

    inline bool endTransfer()
    {
        // wait until all ongoing transmissions are stopped
        TWCR |= (1 << TWSTO);

        TIMEOUT_CHECK(TWCR, TWSTO);

        return true;
    }

    inline void sendByteInt(uint8_t data)
    {
        TWDR = data;
        TWCR &= TWCR_CLR_MASK;
        TWCR |= (1 << TWINT);
    }

    inline void sendStartInt()
    {
        _txBusy = true;
        TWCR &= TWCR_CLR_MASK;
        TWCR |= (1 << TWINT) | (1 << TWSTA) | (1 << TWIE);
    }

    inline void sendStopInt()
    {
        TWCR &= TWCR_CLR_MASK;
        TWCR |= (1 << TWINT) | (1 << TWEA) | (1 << TWSTO);
    }
}    // namespace

namespace Board::I2C
{
    bool init(uint8_t channel, clockSpeed_t speed)
    {
        if (channel >= core::mcu::peripherals::MAX_I2C_INTERFACES)
        {
            return false;
        }

        // no prescaling
        TWSR = 0x00;

        // use formula as per datasheet
        TWBR = ((F_CPU / static_cast<uint32_t>(speed)) - 16) / 2;

        return true;
    }

    bool deInit(uint8_t channel)
    {
        if (channel >= core::mcu::peripherals::MAX_I2C_INTERFACES)
        {
            return false;
        }

        TWCR = 0;
        return true;
    }

    bool write(uint8_t channel, uint8_t address, uint8_t* buffer, size_t size)
    {
        if (channel >= core::mcu::peripherals::MAX_I2C_INTERFACES)
        {
            return false;
        }

        if (size >= I2C_TX_BUFFER_SIZE)
        {
            return false;
        }

        // wait for interface to be ready
        address <<= 1;
        _address = address;

        while (_txBusy)
        {
            ;
        }

        for (size_t i = 0; i < size; i++)
        {
            if (!_txBuffer.insert(buffer[i]))
            {
                return false;
            }
        }

        sendStartInt();

        return true;
    }

    bool deviceAvailable(uint8_t channel, uint8_t address)
    {
        if (channel >= core::mcu::peripherals::MAX_I2C_INTERFACES)
        {
            return false;
        }

        bool found = false;

        if (startTransfer(address))
        {
            found = true;
        }

        if (!endTransfer())
        {
            return false;
        }

        return found;
    }
}    // namespace Board::I2C

ISR(TWI_vect)
{
    switch (TW_STATUS)
    {
    case TW_START:
    case TW_REP_START:
    {
        // send device address
        sendByteInt(_address);
    }
    break;

    // slave address acknowledged
    // data acknowledged
    case TW_MT_SLA_ACK:
    case TW_MT_DATA_ACK:
    {
        uint8_t data;

        if (!_txBuffer.remove(data))
        {
            // transmit stop condition, enable SLA ACK
            sendStopInt();
            _txBusy = false;
        }
        else
        {
            sendByteInt(data);
        }
    }
    break;

    // slave address not acknowledged
    // data not acknowledged
    case TW_MR_SLA_NACK:
    case TW_MT_SLA_NACK:
    case TW_MT_DATA_NACK:
    {
        // transmit stop condition, enable SLA ACK
        sendStopInt();
        _txBusy = false;
    }
    break;

    // bus arbitration lost
    case TW_MT_ARB_LOST:
    {
        // release bus
        TWCR &= TWCR_CLR_MASK;
        TWCR |= (1 << TWINT);
        _txBusy = false;
    }
    break;

    // bus error due to illegal start or stop condition
    case TW_BUS_ERROR:
    {
        // reset internal hardware and release bus
        TWCR &= TWCR_CLR_MASK;
        TWCR |= (1 << TWINT) | (1 << TWSTO) | (1 << TWEA);
        _txBusy = false;
    }
    break;

    case TW_NO_INFO:
    default:
    {
        // nothing to do
    }
    break;
    }
};

#endif