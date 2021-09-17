#pragma once

#include "io/encoders/Filter.h"

class EncodersFilterStub : public IO::Encoders::Filter
{
    public:
    bool isFiltered(size_t                    index,
                    IO::Encoders::position_t  position,
                    IO::Encoders::position_t& filteredPosition,
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