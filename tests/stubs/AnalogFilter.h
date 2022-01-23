#pragma once

#include "io/analog/Filter.h"

class AnalogFilterStub : public IO::Analog::Filter
{
    public:
    AnalogFilterStub()
    {
        for (size_t i = 0; i < IO::Analog::Collection::size(); i++)
            _lastStableValue[i] = 0xFFFF;
    }

    static constexpr adcType_t ADC_RESOLUTION = adcType_t::adc10bit;

    bool isFiltered(size_t index, IO::Analog::type_t type, uint16_t value, uint16_t& filteredValue) override
    {
        filteredValue = value;
        return true;
    }

    uint16_t lastValue(size_t index) override
    {
        if (index < IO::Analog::Collection::size())
            return _lastStableValue[index];

        return 0;
    }

    void reset(size_t index) override
    {
        if (index < IO::Analog::Collection::size())
            _lastStableValue[index] = 0xFFFF;
    }

    void updateLastValue(size_t index, uint16_t value)
    {
        if (index < IO::Analog::Collection::size())
            _lastStableValue[index] = value;
    }

    private:
    uint16_t _lastStableValue[IO::Analog::Collection::size()] = {};
};