/*

Copyright 2015-2022 Igor Petrovic

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
#include "bootloader/updater/Updater.h"
#include "SysExParser/SysExParser.h"
#include "FwSelector/FwSelector.h"
#include "core/MCU.h"

class BTLDRWriter : public Updater::BTLDRWriter
{
    public:
    uint32_t pageSize(size_t index) override
    {
        return board::bootloader::pageSize(index);
    }

    void erasePage(size_t index) override
    {
        board::bootloader::erasePage(index);
    }

    void fillPage(size_t index, uint32_t address, uint32_t value) override
    {
        board::bootloader::fillPage(index, address, value);
    }

    void commitPage(size_t index) override
    {
        board::bootloader::commitPage(index);
    }

    void apply() override
    {
        board::reboot();
    }

    void onFirmwareUpdateStart() override
    {
        board::io::indicators::indicateFirmwareUpdateStart();
    }
};

class HWAFwSelector : public FwSelector::HWA
{
    public:
    uint32_t magicBootValue() override
    {
        return board::bootloader::magicBootValue();
    }

    void setMagicBootValue(uint32_t value) override
    {
        board::bootloader::setMagicBootValue(value);
    }

    void load(FwSelector::fwType_t fwType) override
    {
        switch (fwType)
        {
        case FwSelector::fwType_t::BOOTLOADER:
        {
            board::bootloader::runBootloader();
        }
        break;

        case FwSelector::fwType_t::APPLICATION:
        default:
        {
            board::bootloader::runApplication();
        }
        break;
        }
    }

    void appAddrBoundary(uint32_t& first, uint32_t& last) override
    {
        board::bootloader::appAddrBoundary(first, last);
    }

    bool isHWtriggerActive() override
    {
        return board::bootloader::isHWtriggerActive();
    }

    uint8_t readFlash(uint32_t address) override
    {
        return board::bootloader::readFlash(address);
    }
};

class Reader
{
    public:
    Reader() = default;

    void read()
    {
        uint8_t value = 0;

        if (board::usb::readMIDI(_usbMIDIpacket))
        {
            if (_sysExParser.isValidMessage(_usbMIDIpacket))
            {
                size_t dataSize = _sysExParser.dataBytes();

                if (dataSize)
                {
                    for (size_t i = 0; i < dataSize; i++)
                    {
                        if (_sysExParser.value(i, value))
                        {
                            _updater.feed(value);
                        }
                    }
                }
            }
        }
    }

    private:
    BTLDRWriter           _btldrWriter;
    Updater               _updater = Updater(_btldrWriter, PROJECT_TARGET_UID);
    MIDI::usbMIDIPacket_t _usbMIDIpacket;
    SysExParser           _sysExParser;
};

namespace
{
    HWAFwSelector hwaFwSelector;
    FwSelector    fwSelector(hwaFwSelector);
    Reader        reader;
}    // namespace

int main()
{
    board::init();
    fwSelector.select();

    // everything beyond this point means bootloader is active
    // otherwise jump to other firmware would have already been made

    while (1)
    {
        board::update();
        reader.read();
    }
}