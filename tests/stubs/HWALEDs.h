#pragma once

#include "io/leds/LEDs.h"

class HWALEDsStub : public IO::LEDs::HWA
{
    public:
    HWALEDsStub() {}

    void setState(size_t index, IO::LEDs::brightness_t brightness) override
    {
    }

    size_t rgbSignalIndex(size_t rgbIndex, IO::LEDs::rgbIndex_t rgbComponent) override
    {
        return 0;
    }

    size_t rgbIndex(size_t singleLEDindex) override
    {
        return 0;
    }
};