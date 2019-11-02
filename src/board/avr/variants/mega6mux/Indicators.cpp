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

#include "board/Board.h"
#include "common/OpenDeckMIDIformat/OpenDeckMIDIformat.h"

namespace Board
{
    namespace io
    {
        void ledFlashStartup(bool fwUpdated)
        {
            //there are no indicator leds on this board
            //instead, send special command to USB link which will display indicator led animation

            MIDI::USBMIDIpacket_t packet;

            if (fwUpdated)
                packet.Event = static_cast<uint8_t>(OpenDeckMIDIformat::command_t::fwUpdated);
            else
                packet.Event = static_cast<uint8_t>(OpenDeckMIDIformat::command_t::fwNotUpdated);

            packet.Data1 = 0x00;
            packet.Data2 = 0x00;
            packet.Data3 = 0x00;

            OpenDeckMIDIformat::write(UART_USB_LINK_CHANNEL, packet, OpenDeckMIDIformat::packetType_t::internalCommand);
        }
    }    // namespace io
}    // namespace Board