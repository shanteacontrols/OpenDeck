#pragma once

#include <inttypes.h>
#include "midi/src/MIDI.h"

class SysExParser
{
    public:
    SysExParser() {}

    bool   isValidMessage(MIDI::USBMIDIpacket_t& packet);
    size_t dataBytes();
    bool   value(size_t index, uint8_t& data);

    private:
    bool        parse(MIDI::USBMIDIpacket_t& packet);
    bool        verify();
    static void mergeTo14bit(uint16_t& value, uint8_t high, uint8_t low);

    ///
    /// \brief Enumeration holding USB-specific values for SysEx/System Common messages.
    ///
    /// Normally, USB MIDI CIN (cable index number) is just messageType_t shifted left by four bytes,
    /// however, SysEx/System Common messages have different values so they're grouped in special enumeration.
    ///
    enum class usbMIDIsystemCin_t : uint8_t
    {
        sysCommon1byteCin = 0x50,
        sysCommon2byteCin = 0x20,
        sysCommon3byteCin = 0x30,
        singleByte        = 0xF0,
        sysExStartCin     = 0x40,
        sysExStop1byteCin = sysCommon1byteCin,
        sysExStop2byteCin = 0x60,
        sysExStop3byteCin = 0x70
    };

    ///
    /// \brief Maximum size of SysEx message carrying firmware data.
    /// Two bytes for start/stop bytes
    /// Three bytes for manufacturer ID
    /// 64 bytes for firmware data
    ///
    static const size_t maxFwPacketSize = 2 + 3 + 64;

    ///
    /// \brief Holds decoded SysEx message.
    ///
    uint8_t sysexArray[maxFwPacketSize];

    size_t sysExArrayLength = 0;
};