#include <util/twi.h>
#include "board/Board.h"
#include "core/src/general/Helpers.h"
#include "core/src/general/Timing.h"

#define I2C_TRANSFER_TIMEOUT_MS 10

namespace Board
{
    namespace I2C
    {
        bool init(clockSpeed_t speed)
        {
            //no prescaling
            TWSR = 0x00;

            //use formula as per datasheet
            TWBR = ((F_CPU / static_cast<uint32_t>(speed)) - 16) / 2;

            //enable i2c interface
            TWCR = (1 << TWEN);

            return true;
        }

        bool deInit()
        {
            TWCR = 0;
            return true;
        }

        bool write(uint8_t address, uint8_t* data, size_t size)
        {
            //enable interrupt flag
            //enable start bit (set to master)
            TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);

            //wait for interrupt flag to be cleared
            uint32_t currentTime = core::timing::currentRunTimeMs();

            currentTime = core::timing::currentRunTimeMs();

            while (!BIT_READ(TWCR, TWINT))
            {
                if ((core::timing::currentRunTimeMs() - currentTime) > I2C_TRANSFER_TIMEOUT_MS)
                    return false;
            }

            //check the value of TWI status register
            uint8_t status = TW_STATUS & 0xF8;

            if ((status != TW_START) && (status != TW_REP_START))
                return false;

            //send device address
            TWDR = address;
            TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTA);

            //wait for interrupt flag to be cleared
            currentTime = core::timing::currentRunTimeMs();

            while (!BIT_READ(TWCR, TWINT))
            {
                if ((core::timing::currentRunTimeMs() - currentTime) > I2C_TRANSFER_TIMEOUT_MS)
                    return false;
            }

            status = TW_STATUS & 0xF8;

            if ((status != TW_MT_SLA_ACK) && (status != TW_MR_SLA_ACK))
                return false;

            for (size_t i = 0; i < size; i++)
            {
                TWDR = data[i];
                TWCR = (1 << TWINT) | (1 << TWEN);

                //wait for interrupt flag to be cleared
                while (!BIT_READ(TWCR, TWINT))
                    ;

                currentTime = core::timing::currentRunTimeMs();

                while (!BIT_READ(TWCR, TWINT))
                {
                    if ((core::timing::currentRunTimeMs() - currentTime) > I2C_TRANSFER_TIMEOUT_MS)
                        return false;
                }

                if ((TW_STATUS & 0xF8) != TW_MT_DATA_ACK)
                    return false;
            }

            //wait until all ongoing transmissions are stopped
            TWCR |= (1 << TWSTO);

            currentTime = core::timing::currentRunTimeMs();

            while (!BIT_READ(TWCR, TWSTO))
            {
                if ((core::timing::currentRunTimeMs() - currentTime) > I2C_TRANSFER_TIMEOUT_MS)
                    return false;
            }

            return true;
        }
    }    // namespace I2C
}    // namespace Board