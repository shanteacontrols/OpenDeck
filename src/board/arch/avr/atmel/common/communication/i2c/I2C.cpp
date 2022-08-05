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

#ifdef HW_SUPPORT_I2C

#include "board/Board.h"
#include "core/src/util/Util.h"
#include "core/src/Timing.h"
#include "core/src/util/RingBuffer.h"
#include "core/src/MCU.h"

// note: on AVR, only 1 I2C channel is supported with the index 0
// I2C implementation in core module uses blocking I2C. Here, interrupt
// based implementation is used instead to speed up the transfer.

namespace
{
    constexpr uint8_t                                   TWCR_CLR_MASK = 0x0F;
    core::util::RingBuffer<uint8_t, BUFFER_SIZE_I2C_TX> _txBuffer;
    uint8_t                                             _address;
    volatile bool                                       _txBusy;
    bool                                                _initialized;

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
    initStatus_t init(uint8_t channel, clockSpeed_t speed)
    {
        if (isInitialized(channel))
        {
            return initStatus_t::ALREADY_INIT;
        }

        if (core::mcu::i2c::init(channel, static_cast<uint32_t>(speed)))
        {
            _initialized = true;
            return initStatus_t::OK;
        }

        return initStatus_t::ERROR;
    }

    bool isInitialized(uint8_t channel)
    {
        return _initialized;
    }

    bool deInit(uint8_t channel)
    {
        if (core::mcu::i2c::deInit(channel))
        {
            _initialized = false;
            return true;
        }

        return false;
    }

    bool write(uint8_t channel, uint8_t address, uint8_t* buffer, size_t size)
    {
        if (channel >= CORE_MCU_MAX_I2C_INTERFACES)
        {
            return false;
        }

        if (size >= BUFFER_SIZE_I2C_TX)
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
        return core::mcu::i2c::deviceAvailable(channel, address);
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