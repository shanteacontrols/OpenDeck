#pragma once

#include "application/io/touchscreen/Touchscreen.h"

class HWATouchscreen : public io::Touchscreen::HWA
{
    public:
    HWATouchscreen() = default;

    bool init() override
    {
        return true;
    }

    bool deInit() override
    {
        return true;
    }

    bool write(uint8_t value) override
    {
        return true;
    }

    bool read(uint8_t& value) override
    {
        return true;
    }

    bool allocated(io::common::Allocatable::interface_t interface) override
    {
        return false;
    }
};

class HWATouchscreenCDCPassthrough : public io::Touchscreen::CDCPassthrough
{
    public:
    HWATouchscreenCDCPassthrough() = default;

    bool supported() override
    {
        return true;
    }

    bool init() override
    {
        return true;
    }

    bool deInit() override
    {
        return true;
    }

    bool uartRead(uint8_t& value) override
    {
        return true;
    }

    bool uartWrite(uint8_t value) override
    {
        return true;
    }

    bool cdcRead(uint8_t* buffer, size_t& size, const size_t maxSize) override
    {
        return true;
    }

    bool cdcWrite(uint8_t* buffer, size_t size) override
    {
        return true;
    }

    bool allocated(io::common::Allocatable::interface_t interface) override
    {
        return false;
    }
};