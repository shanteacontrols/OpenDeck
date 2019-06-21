/*

Copyright 2015-2019 Igor Petrovic

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

#include "DataTypes.h"
#include "midi/src/MIDI.h"

namespace OpenDeckMIDIformat
{
    ///
    /// \brief Used to read data using custom OpenDeck format from UART interface.
    /// @param [in] channel         UART channel on MCU.
    /// @param [in] USBMIDIpacket   Pointer to structure holding MIDI data being read.
    /// @param [in] packetType      Pointer to variable in which read packet type is being stored.
    /// \returns True if data is available, false otherwise.
    ///
    bool read(uint8_t channel, MIDI::USBMIDIpacket_t& USBMIDIpacket, odPacketType_t& packetType);

    ///
    /// \brief Used to write data using custom OpenDeck format to UART interface.
    /// @param [in] channel         UART channel on MCU.
    /// @param [in] USBMIDIpacket   Pointer to structure holding MIDI data to write.
    /// @param [in] packetType      Type of OpenDeck packet to send.
    /// \returns True on success, false otherwise.
    ///
    bool write(uint8_t channel, MIDI::USBMIDIpacket_t& USBMIDIpacket, odPacketType_t packetType);
}