#pragma once

#include "application/io/analog/Filter.h"

class AnalogFilterStub : public io::Analog::Filter
{
    public:
    AnalogFilterStub()
    {
        for (size_t i = 0; i < io::Analog::Collection::SIZE(); i++)
        {
            _lastStableValue[i] = 0xFFFF;
        }
    }

    bool isFiltered(size_t index, descriptor_t& descriptor) override
    {
        return true;
    }

    void reset(size_t index) override
    {
        if (index < io::Analog::Collection::SIZE())
        {
            _lastStableValue[index] = 0xFFFF;
        }
    }

    private:
    uint16_t _lastStableValue[io::Analog::Collection::SIZE()] = {};
};