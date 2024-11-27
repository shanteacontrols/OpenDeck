#pragma once

#include "application/io/buttons/Filter.h"

class ButtonsFilterStub : public io::Buttons::Filter
{
    public:
    bool isFiltered(size_t index, uint8_t& numberOfReadings, uint16_t& states) override
    {
        return true;
    }
};