#pragma once

#include "io/analog/Analog.h"

class HWAAnalog : public io::Analog::HWA
{
    public:
    HWAAnalog() = default;

    MOCK_METHOD2(value, bool(size_t index, uint16_t& value));
};