/*

Copyright 2015-2021 Igor Petrovic

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*/

#pragma once

#include <stddef.h>
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
    /// Enumeration holding USB-specific values for SysEx/System Common messages.
    /// Normally, USB MIDI CIN (cable index number) is just messageType_t shifted left by four bytes,
    /// however, SysEx/System Common messages have different values so they're grouped in special enumeration.
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

    bool        parse(MIDI::USBMIDIpacket_t& packet);
    bool        verify();
    static void mergeTo14bit(uint16_t& value, uint8_t high, uint8_t low);

    /// Maximum size of SysEx message carrying firmware data.
    /// Two bytes for start/stop bytes
    /// Three bytes for manufacturer ID
    /// 64 bytes for firmware data
    static constexpr size_t MAX_FW_PACKET_SIZE = 2 + 3 + 64;

    /// Byte index in SysEx message on which firmware data starts.
    static constexpr size_t DATA_START_BYTE = 4;

    /// Holds decoded SysEx message.
    uint8_t _sysExArray[MAX_FW_PACKET_SIZE];

    size_t _sysExArrayLength = 0;
};