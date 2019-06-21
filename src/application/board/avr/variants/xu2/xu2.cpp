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

#include <avr/interrupt.h>
#include "board/Board.h"
#include "../../../../../common/OpenDeckMIDIformat/OpenDeckMIDIformat.h"

namespace
{
    MIDI::USBMIDIpacket_t USBMIDIpacket;
    odPacketType_t        packetType;
}    // namespace

int main(void)
{
    Board::init();

    sei();

    while (1)
    {
        if (Board::USB::readMIDI(USBMIDIpacket))
            OpenDeckMIDIformat::write(UART_USB_LINK_CHANNEL, USBMIDIpacket, odPacketType_t::packetMIDI);

        if (OpenDeckMIDIformat::read(UART_USB_LINK_CHANNEL, USBMIDIpacket, packetType))
        {
            if (packetType != odPacketType_t::packetIntCMD)
                Board::USB::writeMIDI(USBMIDIpacket);
        }
    }
}