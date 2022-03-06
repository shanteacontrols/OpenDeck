#pragma once

#include "io/buttons/Buttons.h"

class HWAButtons : public IO::Buttons::HWA
{
    public:
    HWAButtons() = default;

    MOCK_METHOD3(state, bool(size_t index, uint8_t& numberOfReadings, uint32_t& states));

    size_t buttonToEncoderIndex(size_t index) override
    {
        return index / 2;
    }
};