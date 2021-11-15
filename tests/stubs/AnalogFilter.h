#pragma once

#include "io/analog/Filter.h"

class AnalogFilterStub : public IO::Analog::Filter
{
    public:
    AnalogFilterStub() {}

    static constexpr adcType_t ADC_RESOLUTION = adcType_t::adc10bit;

    bool isFiltered(size_t index, IO::Analog::type_t type, uint16_t value, uint16_t& filteredValue) override
    {
        filteredValue = value;
        return true;
    }

    void reset(size_t index) override
    {
    }
};