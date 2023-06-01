#pragma once

#include "application/io/i2c/I2C.h"

class HWADisplay : public io::I2C::Peripheral::HWA
{
    public:
    HWADisplay() = default;

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