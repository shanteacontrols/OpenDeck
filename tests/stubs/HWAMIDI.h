#pragma once

#include "midi/src/MIDI.h"

class HWAMIDIStub : public MIDI::HWA
{
    public:
    HWAMIDIStub() = default;

    bool init(MIDI::interface_t interface) override
    {
        return true;
    }

    bool deInit(MIDI::interface_t interface) override
    {
        return true;
    }

    bool dinRead(uint8_t& data) override
    {
        return false;
    }

    bool dinWrite(uint8_t data) override
    {
        return false;
    }

    bool usbRead(MIDI::usbMIDIPacket_t& packet) override
    {
        return false;
    }

    bool usbWrite(MIDI::usbMIDIPacket_t& packet) override
    {
        usbPacket.push_back(packet);
        return true;
    }

    std::vector<MIDI::usbMIDIPacket_t> usbPacket;
};