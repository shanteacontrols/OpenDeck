#pragma once

#include "io/leds/LEDs.h"

class HWALEDs : public IO::LEDs::HWA
{
    public:
    HWALEDs() = default;

    MOCK_METHOD2(setState, void(size_t index, IO::LEDs::brightness_t brightness));

    size_t rgbSignalIndex(size_t rgbIndex, IO::LEDs::rgbIndex_t rgbComponent) override
    {
        return rgbIndex * 3 + static_cast<uint8_t>(rgbComponent);
    }

    size_t rgbIndex(size_t singleLEDindex) override
    {
        return singleLEDindex / 3;
    }
};