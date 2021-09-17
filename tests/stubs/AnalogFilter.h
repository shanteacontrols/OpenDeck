#pragma once

#include "io/analog/Filter.h"

class AnalogFilterStub : public IO::Analog::Filter
{
    public:
    AnalogFilterStub() {}

    IO::Analog::adcType_t adcType() override
    {
#ifdef ADC_12_BIT
        return IO::Analog::adcType_t::adc12bit;
#else
        return IO::Analog::adcType_t::adc10bit;
#endif
    }

    bool isFiltered(size_t index, IO::Analog::type_t type, uint16_t value, uint16_t& filteredValue) override
    {
        filteredValue = value;
        return true;
    }

    void reset(size_t index) override
    {
    }
};