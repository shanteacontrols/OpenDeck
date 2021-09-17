#pragma once

#include "io/display/U8X8/U8X8.h"

class HWAU8X8Stub : public IO::U8X8::HWAI2C
{
    public:
    HWAU8X8Stub() {}

    bool init() override
    {
        return false;
    }

    bool deInit() override
    {
        return false;
    }

    bool write(uint8_t address, uint8_t* buffer, size_t size) override
    {
        return false;
    }
};