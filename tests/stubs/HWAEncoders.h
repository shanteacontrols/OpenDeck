#pragma once

#include "io/encoders/Encoders.h"

class HWAEncoders : public IO::Encoders::HWA
{
    public:
    HWAEncoders() = default;

    MOCK_METHOD3(state, bool(size_t index, uint8_t& numberOfReadings, uint32_t& states));
};