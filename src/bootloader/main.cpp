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

#include "Config.h"
#include "board/Board.h"
#include "updater/Updater.h"
#if defined(USB_MIDI_SUPPORTED)
#include "SysExParser/SysExParser.h"
#include "application/OpenDeck/sysconfig/Constants.h"
#endif

class BTLDRWriter : public Bootloader::Updater::BTLDRWriter
{
    public:
    size_t pageSize(size_t index) override
    {
        return Board::bootloader::pageSize(index);
    }

    void erasePage(uint32_t address) override
    {
        Board::bootloader::erasePage(address);
    }

    void fillPage(uint32_t address, uint16_t data) override
    {
        Board::bootloader::fillPage(address, data);
    }

    void writePage(uint32_t address) override
    {
        Board::bootloader::writePage(address);
    }

    void apply() override
    {
        Board::bootloader::applyFw();
    }
};

namespace
{
#ifndef USB_LINK_MCU
    BTLDRWriter         btldrWriter;
    Bootloader::Updater updater(btldrWriter, COMMAND_FW_UPDATE_START);
#endif

#if defined(USB_MIDI_SUPPORTED)
    MIDI::USBMIDIpacket_t usbMIDIpacket;
    SysExParser           sysExParser;
#endif
}    // namespace

int main()
{
    Board::init();

    while (1)
    {
#ifndef USB_MIDI_SUPPORTED
        //read data from uart
        uint8_t data = 0;

        if (Board::UART::read(UART_USB_LINK_CHANNEL, data))
        {
            updater.feed(data);
        }
#else
        if (Board::USB::readMIDI(usbMIDIpacket))
        {
            if (sysExParser.isValidMessage(usbMIDIpacket))
            {
                size_t  dataSize = sysExParser.dataBytes();
                uint8_t data     = 0;

                if (dataSize)
                {
                    for (size_t i = 0; i < dataSize; i++)
                    {
                        if (sysExParser.value(i, data))
                        {
#ifndef USB_LINK_MCU
                            updater.feed(data);
#else
                            Board::UART::write(UART_USB_LINK_CHANNEL, data);
#endif
                        }
                    }
                }
            }
        }
#endif
    }
}