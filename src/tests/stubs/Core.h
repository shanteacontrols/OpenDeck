#pragma once

#include <inttypes.h>

#ifdef __cplusplus
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
#endif