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

    bool usbRead(MIDI::USBMIDIpacket_t& USBMIDIpacket) override
    {
        return false;
    }

    bool usbWrite(MIDI::USBMIDIpacket_t& USBMIDIpacket) override
    {
        midiPacket.push_back(USBMIDIpacket);
        return true;
    }

    std::vector<MIDI::USBMIDIpacket_t> midiPacket;
};