#pragma once

#include "application/system/System.h"

class HWASystem : public sys::Instance::HWA
{
    public:
    HWASystem() = default;

    bool init() override
    {
        return true;
    }

    void update() override
    {
    }

    void reboot(FwSelector::fwType_t type) override
    {
    }

    void registerOnUSBconnectionHandler(sys::Instance::usbConnectionHandler_t&& usbConnectionHandler) override
    {
    }
};