/*

Copyright Igor Petrovic

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

#include "application/protocol/midi/common.h"

namespace sysex_parser
{
    class SysExParser
    {
        public:
        SysExParser() = default;

        bool   isValidMessage(protocol::midi::UsbPacket& packet);
        size_t dataBytes();
        bool   value(size_t index, uint8_t& data);

        private:
        /// Enumeration holding USB-specific values for SysEx/System Common messages.
        /// Normally, USB MIDI CIN (cable index number) is just messageType_t shifted left by four bytes,
        /// however, SysEx/System Common messages have different values so they're grouped in special enumeration.
        enum class usbMidiSystemCin_t : uint8_t
        {
            SYS_COMMON1BYTE_CIN  = 0x50,
            SYS_COMMON2BYTE_CIN  = 0x20,
            SYS_COMMON3BYTE_CIN  = 0x30,
            SINGLE_BYTE          = 0xF0,
            SYS_EX_START_CIN     = 0x40,
            SYS_EX_STOP1BYTE_CIN = SYS_COMMON1BYTE_CIN,
            SYS_EX_STOP2BYTE_CIN = 0x60,
            SYS_EX_STOP3BYTE_CIN = 0x70
        };

        /// Maximum size of SysEx message carrying firmware data.
        /// Two bytes for start/stop bytes
        /// Three bytes for manufacturer ID
        /// 64 bytes for firmware data
        static constexpr size_t MAX_FW_PACKET_SIZE = 2 + 3 + 64;

        /// Byte index in SysEx message on which firmware data starts.
        static constexpr size_t DATA_START_BYTE = 4;

        uint8_t _sysExArray[MAX_FW_PACKET_SIZE] = {};
        size_t  _sysExArrayLength               = 0;

        bool parse(protocol::midi::UsbPacket& packet);
        bool verify();
    };
}    // namespace sysex_parser
