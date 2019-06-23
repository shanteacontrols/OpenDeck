#pragma once

#include <inttypes.h>

#define CORE_ARCH avr

#ifdef __cplusplus
namespace core
{
    namespace avr
    {
        namespace i2c
        {
            enum class transferType_t : uint8_t
            {
                write,
                read
            };

            inline void enable()
            {
            }

            inline bool write(uint8_t data)
            {
                return true;
            }

            inline bool startComm(uint8_t address, transferType_t type)
            {
                return true;
            }

            inline void stopComm()
            {
            }
        }    // namespace i2c
    }        // namespace avr
}    // namespace core
#endif