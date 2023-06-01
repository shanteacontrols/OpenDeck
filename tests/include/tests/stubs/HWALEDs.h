#pragma once

#include "application/io/leds/LEDs.h"

class HWALEDs : public io::LEDs::HWA
{
    public:
    HWALEDs() = default;

    MOCK_METHOD2(setState, void(size_t index, io::LEDs::brightness_t brightness));

    size_t rgbComponentFromRGB(size_t index, io::LEDs::rgbComponent_t component) override
    {
        return index * 3 + static_cast<uint8_t>(component);
    }

    size_t rgbFromOutput(size_t index) override
    {
        return index / 3;
    }
};