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
#include "updater/Updater.h"
#include "SysExParser/SysExParser.h"
#include "FwSelector/FwSelector.h"
#include "core/src/Timing.h"
#include <Target.h>

namespace
{
#ifdef HW_USB_OVER_SERIAL
    /// Value sent from non-USB target to USB link before loading application
    constexpr uint8_t USB_LINK_MAGIC_VAL_APP = 0x55;

    /// Value sent from non-USB target to USB link after the firmware update is started
    constexpr uint8_t TARGET_FW_UPDATE_STARTED = 0xFC;

    /// Value sent from non-USB target to USB link after the firmware update is done
    constexpr uint8_t TARGET_FW_UPDATE_DONE = 0xFD;
#endif
}    // namespace

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
#ifndef HW_SUPPORT_USB
        board::uart::write(HW_UART_CHANNEL_USB_LINK, TARGET_FW_UPDATE_DONE);
#endif
        board::reboot();
    }

    void onFirmwareUpdateStart() override
    {
#ifndef HW_SUPPORT_USB
        board::uart::write(HW_UART_CHANNEL_USB_LINK, TARGET_FW_UPDATE_STARTED);
#endif

        board::io::indicators::indicateFirmwareUpdateStart();
    }
};

class HWAFwSelector : public FwSelector::HWA
{
    public:
    uint8_t magicBootValue() override
    {
        return board::bootloader::magicBootValue();
    }

    void setMagicBootValue(uint8_t value) override
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
#if !defined(HW_SUPPORT_USB)
            // on non-usb supported board, send magic value to USB link so that link
            // knows whether the target MCU has entered application
            // if link MCU doesn't receive this, bootloader should be entered
            board::uart::write(HW_UART_CHANNEL_USB_LINK, USB_LINK_MAGIC_VAL_APP);
            core::timing::waitMs(1);
            board::bootloader::runApplication();
#elif defined(HW_USB_OVER_SERIAL_HOST)
            // wait a bit first
            core::timing::waitMs(1500);
            uint8_t data;

            while (board::uart::read(HW_UART_CHANNEL_USB_LINK, data))
            {
                if (data == USB_LINK_MAGIC_VAL_APP)
                {
                    // everything fine, proceed with app load
                    board::bootloader::runApplication();
                }
            }

            board::bootloader::runBootloader();
#else
            board::bootloader::runApplication();
#endif
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

#ifdef HW_SUPPORT_USB
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
#ifdef HW_USB_OVER_SERIAL_HOST
                            board::uart::write(HW_UART_CHANNEL_USB_LINK, value);

                            // expect ACK but ignore the value
                            while (!board::uart::read(HW_UART_CHANNEL_USB_LINK, value))
                            {
                                ;
                            }
#else
                            _updater.feed(value);
#endif
                        }
                    }
                }
            }
        }

#ifdef HW_USB_OVER_SERIAL_HOST
        if (board::uart::read(HW_UART_CHANNEL_USB_LINK, value))
        {
            if (value == TARGET_FW_UPDATE_DONE)
            {
                // To avoid compiling the entire parser to figure out the end
                // of the FW stream (if won't fit into 4k space), wait for TARGET_FW_UPDATE_DONE
                // byte on UART sent by target MCU (this is done in BTLDRWriter::apply())
                board::reboot();
            }
            else if (value == TARGET_FW_UPDATE_STARTED)
            {
                board::io::indicators::indicateFirmwareUpdateStart();
            }
        }
#endif
    }
#else
    void read()
    {
        uint8_t value;

        if (board::uart::read(HW_UART_CHANNEL_USB_LINK, value))
        {
            // send USB_LINK_MAGIC_VAL_APP for ACK so that USB link can proceed with next byte
            board::uart::write(HW_UART_CHANNEL_USB_LINK, USB_LINK_MAGIC_VAL_APP);
            _updater.feed(value);
        }
    }
#endif

    private:
#ifndef HW_USB_OVER_SERIAL_HOST
    BTLDRWriter _btldrWriter;
    Updater     _updater = Updater(_btldrWriter, FW_UID);
#endif

#ifdef HW_SUPPORT_USB
    MIDI::usbMIDIPacket_t _usbMIDIpacket;
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