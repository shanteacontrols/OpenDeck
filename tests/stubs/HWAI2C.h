#pragma once

#include "io/i2c/I2C.h"

class HWAI2C : public IO::I2C::HWA
{
    public:
    HWAI2C() = default;

    bool init() override
    {
        return true;
    }

    bool write(uint8_t address, uint8_t* buffer, size_t size) override
    {
        return true;
    }

    bool deviceAvailable(uint8_t address) override
    {
        return false;
    }
};