#pragma once

#include "protocol/dmx/DMX.h"

class HWADMX : public Protocol::DMX::HWA
{
    public:
    HWADMX() = default;

    bool init(DMXUSBWidget::dmxBuffer_t& buffer) override
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

    void updateBuffer(DMXUSBWidget::dmxBuffer_t& buffer) override
    {
    }

    bool uniqueID(core::mcu::uniqueID_t& uniqueID) override
    {
        return true;
    }

    bool allocated(IO::Common::Allocatable::interface_t interface) override
    {
        return false;
    }
};