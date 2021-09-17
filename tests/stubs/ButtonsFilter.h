#pragma once

#include "io/buttons/Filter.h"

class ButtonsFilterStub : public IO::Buttons::Filter
{
    public:
    bool isFiltered(size_t index, uint8_t& numberOfReadings, uint32_t& states) override
    {
        return true;
    }
};