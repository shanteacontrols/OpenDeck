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
#include "updater/Updater.h"
#include "SysExParser/SysExParser.h"
#include "midi/src/Helpers.h"

class BTLDRWriter : public Bootloader::Updater::BTLDRWriter
{
    public:
    uint32_t pageSize(size_t index) override
    {
        return Board::bootloader::pageSize(index);
    }

    void erasePage(size_t index) override
    {
        Board::bootloader::erasePage(index);
    }

    void fillPage(size_t index, uint32_t address, uint16_t data) override
    {
        Board::bootloader::fillPage(index, address, data);
    }

    void writePage(size_t index) override
    {
        Board::bootloader::writePage(index);
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
    Bootloader::Updater updater(btldrWriter, COMMAND_FW_UPDATE_START, COMMAND_FW_UPDATE_END, FW_UID);
#else
    uint8_t endCounter;
#endif

#if defined(USB_MIDI_SUPPORTED)
    MIDI::USBMIDIpacket_t usbMIDIpacket;

    //special midi sysex message/packet which should be sent back to host
    //single byte of data only so that it fits into single USB MIDI packet and also
    //to avoid having complex sysex sending mechanism in bootloader
    const MIDI::USBMIDIpacket_t usbMIDIpacketEcho = {
        .Event = GET_USB_MIDI_EVENT(0, static_cast<uint8_t>(MIDI::usbMIDIsystemCin_t::sysExStop3byteCin)),    //event indicating sysex with single byte of data
        .Data1 = 0xF0,
        .Data2 = 0x55,
        .Data3 = 0xF7
    };

    SysExParser sysExParser;
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

        if (Board::UART::read(UART_CHANNEL_USB_LINK, data))
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
                            Board::UART::write(UART_CHANNEL_USB_LINK, data);
                            //to avoid compiling the entire parser to figure out the end
                            //of the fw stream (if won't fit into 4k space), parse fw end manually
                            //fw end stream is detected by two messages with data size being 2
                            //taken those 4 bytes together, COMMAND_FW_UPDATE_END value should be formed
                            //if it is, reboot the mcu into app mode

                            if (dataSize == 2)
                            {
                                if (((COMMAND_FW_UPDATE_END >> (endCounter * 8)) & static_cast<uint32_t>(0xFF)) != data)
                                {
                                    endCounter = 0;
                                }
                                else
                                {
                                    endCounter++;

                                    if (endCounter == 4)
                                    {
                                        while (!Board::UART::isTxEmpty(UART_CHANNEL_USB_LINK))
                                            ;

                                        //this will actually flash the integrated LEDs and reset the MCU
                                        Board::bootloader::applyFw();
                                    }
                                }
                            }
                            else
                            {
                                endCounter = 0;
                            }
#endif
                        }
                    }
                }
            }
            else
            {
                if (
                    (usbMIDIpacket.Event == usbMIDIpacketEcho.Event) &&
                    (usbMIDIpacket.Data1 == usbMIDIpacketEcho.Data1) &&
                    (usbMIDIpacket.Data2 == usbMIDIpacketEcho.Data2) &&
                    (usbMIDIpacket.Data3 == usbMIDIpacketEcho.Data3))
                {
                    //return the message back to host
                    Board::USB::writeMIDI(usbMIDIpacket);
                }
            }
        }
#endif
    }
}