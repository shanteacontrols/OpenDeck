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

    bool readUSB(uint8_t* buffer, size_t& size, const size_t maxSize) override
    {
        return true;
    }

    bool writeUSB(uint8_t* buffer, size_t size) override
    {
        return true;
    }

    bool updateChannel(uint16_t channel, uint8_t value) override
    {
        return true;
    }

    void packetComplete() override
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