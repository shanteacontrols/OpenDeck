#pragma once

#include "io/i2c/I2C.h"

class HWAI2CStub : public IO::I2C::HWA
{
    public:
    HWAI2CStub() = default;

    bool init() override
    {
        return false;
    }

    bool write(uint8_t address, uint8_t* buffer, size_t size) override
    {
        return false;
    }

    bool deviceAvailable(uint8_t address) override
    {
        return false;
    }
};