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

#include "board/Board.h"
#include "board/common/USBMIDIOverSerial/USBMIDIOverSerial.h"
#include "Commands.h"

namespace
{
    MIDI::USBMIDIpacket_t                  USBMIDIpacket;
    Board::USBMIDIOverSerial::packetType_t packetType;
}    // namespace

int main(void)
{
    Board::init();

    while (1)
    {
        //USB -> UART
        if (Board::USB::readMIDI(USBMIDIpacket))
            Board::USBMIDIOverSerial::write(UART_CHANNEL_USB_LINK, USBMIDIpacket, Board::USBMIDIOverSerial::packetType_t::midi);

        //UART -> USB
        if (Board::USBMIDIOverSerial::read(UART_CHANNEL_USB_LINK, USBMIDIpacket, packetType))
        {
            if (packetType == Board::USBMIDIOverSerial::packetType_t::midi)
            {
                Board::USB::writeMIDI(USBMIDIpacket);
            }
            else
            {
                //internal command
                if (USBMIDIpacket.Event == static_cast<uint8_t>(USBLink::internalCMD_t::rebootBTLDR))
                {
                    //use received data as the magic bootloader value
                    Board::bootloader::setMagicBootValue(USBMIDIpacket.Data1);
                    Board::reboot();
                }
            }
        }
    }
}