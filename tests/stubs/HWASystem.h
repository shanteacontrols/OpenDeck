#pragma once

#include "system/System.h"

class HWASystem : public System::Instance::HWA
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

    void registerOnUSBconnectionHandler(System::Instance::usbConnectionHandler_t&& usbConnectionHandler) override
    {
    }
};