#pragma once

#include "io/analog/Filter.h"

class AnalogFilterStub : public IO::Analog::Filter
{
    public:
    AnalogFilterStub()
    {
        for (size_t i = 0; i < IO::Analog::Collection::size(); i++)
        {
            _lastStableValue[i] = 0xFFFF;
        }
    }

    bool isFiltered(size_t index, descriptor_t& descriptor) override
    {
        return true;
    }

    uint16_t lastValue(size_t index) override
    {
        if (index < IO::Analog::Collection::size())
        {
            return _lastStableValue[index];
        }

        return 0;
    }

    void reset(size_t index) override
    {
        if (index < IO::Analog::Collection::size())
        {
            _lastStableValue[index] = 0xFFFF;
        }
    }

    void updateLastValue(size_t index, uint16_t value)
    {
        if (index < IO::Analog::Collection::size())
        {
            _lastStableValue[index] = value;
        }
    }

    private:
    uint16_t _lastStableValue[IO::Analog::Collection::size()] = {};
};