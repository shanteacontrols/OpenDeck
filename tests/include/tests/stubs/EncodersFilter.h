#pragma once

#include "io/encoders/Filter.h"

class EncodersFilterStub : public io::Encoders::Filter
{
    public:
    bool isFiltered(size_t                    index,
                    io::Encoders::position_t  position,
                    io::Encoders::position_t& filteredPosition,
                    uint32_t                  sampleTakenTime) override
    {
        return true;
    }

    void reset(size_t index) override
    {
    }

    uint32_t lastMovementTime(size_t index) override
    {
        return 0;
    }
};