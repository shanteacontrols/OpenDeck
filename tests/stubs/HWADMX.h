#pragma once

#include "protocol/dmx/DMX.h"

class HWADMX : public Protocol::DMX::HWA
{
    public:
    HWADMX() = default;

    bool init() override
    {
        return true;
    }

    bool deInit() override
    {
        return true;
    }

    bool readUSB(DMXUSBWidget::usbReadBuffer_t& buffer, size_t& size) override
    {
        return true;
    }

    bool writeUSB(uint8_t* buffer, size_t size) override
    {
        return true;
    }

    void setBuffer(DMXUSBWidget::dmxBuffer_t& buffer) override
    {
    }

    bool uniqueID(Protocol::DMX::uniqueID_t& uniqueID) override
    {
        return true;
    }

    bool allocated(IO::Common::interface_t interface) override
    {
        return false;
    }
};