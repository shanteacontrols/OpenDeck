/*

Copyright 2015-2020 Igor Petrovic

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

namespace
{
    MIDI::USBMIDIpacket_t            USBMIDIpacket;
    OpenDeckMIDIformat::packetType_t packetType;
}    // namespace

int main(void)
{
    Board::init();

    while (1)
    {
        if (Board::USB::readMIDI(USBMIDIpacket))
            OpenDeckMIDIformat::write(UART_USB_LINK_CHANNEL, USBMIDIpacket, OpenDeckMIDIformat::packetType_t::midi);

        if (OpenDeckMIDIformat::read(UART_USB_LINK_CHANNEL, USBMIDIpacket, packetType))
        {
            if (packetType != OpenDeckMIDIformat::packetType_t::internalCommand)
                Board::USB::writeMIDI(USBMIDIpacket);
        }
    }
}