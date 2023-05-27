#pragma once

#include "system/Builder.h"

class HWAAnalog : public ::sys::Builder::HWA::IO::Analog
{
    public:
    HWAAnalog() = default;

    uint8_t adcBits() override
    {
        return 10;    // unused in tests
    }

    MOCK_METHOD2(value, bool(size_t index, uint16_t& value));
};