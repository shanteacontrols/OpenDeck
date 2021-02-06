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
#include "updater/Updater.h"
#include "SysExParser/SysExParser.h"
#include "midi/src/MIDI.h"
#include "FwSelector/FwSelector.h"
#include "core/src/general/Timing.h"

//Number of sent FW bytes via USB link after which confirmation is expected
#define FW_CHUNK_BUFFER_SIZE_ACK 16

class BTLDRWriter : public Updater::BTLDRWriter
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
        Board::reboot();
    }
};

class HWAFwSelector : public FwSelector::HWA
{
    public:
    uint8_t magicBootValue() override
    {
        return Board::bootloader::magicBootValue();
    }

    void setMagicBootValue(uint8_t value) override
    {
        Board::bootloader::setMagicBootValue(value);
    }

    void load(FwSelector::fwType_t fwType) override
    {
        switch (fwType)
        {
        case FwSelector::fwType_t::bootloader:
            Board::bootloader::runBootloader();
            break;

        case FwSelector::fwType_t::application:
            Board::bootloader::runApplication();
            break;

        case FwSelector::fwType_t::cdc:
            Board::bootloader::runCDC();
            break;
        }
    }

    void appAddrBoundary(uint32_t& first, uint32_t& last) override
    {
        Board::bootloader::appAddrBoundary(first, last);
    }

    bool isHWtriggerActive() override
    {
        return Board::bootloader::isHWtriggerActive();
    }

    uint8_t readFlash(uint32_t address) override
    {
        return Board::bootloader::readFlash(address);
    }
};

class Reader
{
    public:
    Reader() = default;

    void init()
    {
#if defined(USB_LINK_MCU) || !defined(USB_MIDI_SUPPORTED)
        //make sure all incoming data is cleared on startup to avoid junk values
        uint8_t data;
        core::timing::waitMs(500);

        while (Board::UART::read(UART_CHANNEL_USB_LINK, data))
            ;
#endif
    }

#ifdef USB_MIDI_SUPPORTED
    void read()
    {
        if (Board::USB::readMIDI(_usbMIDIpacket))
        {
            if (_sysExParser.isValidMessage(_usbMIDIpacket))
            {
                size_t  dataSize = _sysExParser.dataBytes();
                uint8_t data     = 0;

                if (dataSize)
                {
                    for (size_t i = 0; i < dataSize; i++)
                    {
                        if (_sysExParser.value(i, data))
                        {
#ifdef USB_LINK_MCU
                            Board::UART::write(UART_CHANNEL_USB_LINK, data);

                            //To avoid compiling the entire parser to figure out the end
                            //of the FW stream (if won't fit into 4k space), parse FW end manually.
                            //FW end stream is detected by two messages with data size being 2.
                            //Taken those 4 bytes together, COMMAND_FW_UPDATE_END value should be formed
                            //If it is, reboot the MCU into app mode.

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
                                        //end of stream detected
                                        //wait until everything is sent and then reboot
                                        while (!Board::UART::isTxEmpty(UART_CHANNEL_USB_LINK))
                                            ;

                                        Board::reboot();
                                    }
                                }
                            }
                            else
                            {
                                endCounter = 0;
                            }

                            if (++fwChunkCounter == FW_CHUNK_BUFFER_SIZE_ACK)
                            {
                                fwChunkCounter = 0;

                                //target MCU might be busy writing new flash page - packets could be missed
                                //expect a byte to appear on RX as confirmation
                                //value doesn't matter
                                while (!Board::UART::read(UART_CHANNEL_USB_LINK, data))
                                    ;
                            }
#else
                            _updater.feed(data);
#endif
                        }
                    }
                }
            }
        }
    }
#else
    void read()
    {
        //read data from uart
        uint8_t        data    = 0;
        static uint8_t counter = 0;

        if (Board::UART::read(UART_CHANNEL_USB_LINK, data))
        {
            if (++counter == FW_CHUNK_BUFFER_SIZE_ACK)
            {
                //send confirmation to USB link MCU
                counter = 0;
                Board::UART::write(UART_CHANNEL_USB_LINK, 0xFF);
            }

            _updater.feed(data);
        }
    }
#endif

    private:
#ifndef USB_LINK_MCU
    BTLDRWriter _btldrWriter;
    Updater     _updater = Updater(_btldrWriter, COMMAND_FW_UPDATE_START, COMMAND_FW_UPDATE_END, FW_UID);
#else
    uint8_t fwChunkCounter = 0;
    uint8_t endCounter;
#endif

#ifdef USB_MIDI_SUPPORTED
    MIDI::USBMIDIpacket_t _usbMIDIpacket;
    SysExParser           _sysExParser;
#endif
};

namespace
{
    HWAFwSelector hwaFwSelector;
    FwSelector    fwSelector(hwaFwSelector);
    Reader        reader;
}    // namespace

int main()
{
    Board::init();
    reader.init();
    fwSelector.init();

    //everything beyond this point means bootloader is active
    //otherwise jump to other firmware would have already been made

    while (1)
    {
        reader.read();
    }
}