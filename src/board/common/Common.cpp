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
#include "board/Internal.h"
#include "common/OpenDeckMIDIformat/OpenDeckMIDIformat.h"
#include "core/src/general/Reset.h"
#include "core/src/general/Timing.h"

//holds total flash size - inserted in the binary by build process
//address where this variable is stored contains total firmware length
//after the last firmware address, crc of firmware is stored
//this is used by the bootloader to verify the crc of application
uint32_t flashSize __attribute__((section(".fwMetadata"))) __attribute__((used)) = 0;

namespace core
{
    namespace timing
    {
        namespace detail
        {
            ///
            /// \brief Implementation of core variable used to keep track of run time in milliseconds.
            ///
            volatile uint32_t rTime_ms;
        }    // namespace detail
    }        // namespace timing
}    // namespace core

namespace Board
{
    void init()
    {
#ifdef FW_APP
        detail::setup::application();
#else
        detail::setup::bootloader();

        bool runBootloader = true;

        if (detail::bootloader::btldrTrigger() == detail::bootloader::btldrTrigger_t::none)
        {
            if (detail::bootloader::isAppValid())
                runBootloader = false;
            else
                runBootloader = true;
        }
        else
        {
            runBootloader = true;
        }

        if (runBootloader)
        {
            detail::bootloader::runBootloader();
        }
        else
        {
#ifdef LED_INDICATORS
            detail::io::ledFlashStartup(false);
#endif
            detail::bootloader::runApplication();
        }
#endif
    }

    void reboot(rebootType_t type)
    {
        switch (type)
        {
        case rebootType_t::rebootApp:
            detail::bootloader::clearSWtrigger();
#ifndef USB_MIDI_SUPPORTED
            //signal to usb link to reboot as well
            //no need to do this for bootloader reboot - the bootloader already sends btldrReboot command to USB link
            MIDI::USBMIDIpacket_t USBMIDIpacket;

            USBMIDIpacket.Event = static_cast<uint8_t>(OpenDeckMIDIformat::command_t::appReboot);
            USBMIDIpacket.Data1 = 0x00;
            USBMIDIpacket.Data2 = 0x00;
            USBMIDIpacket.Data3 = 0x00;

            OpenDeckMIDIformat::write(UART_CHANNEL_USB_LINK, USBMIDIpacket, OpenDeckMIDIformat::packetType_t::internalCommand);
            while (!Board::UART::isTxEmpty(UART_CHANNEL_USB_LINK))
                ;

            //give some time to usb link to properly re-initialize so that everything is in sync
            core::timing::waitMs(50);
#endif
            break;

        case rebootType_t::rebootBtldr:
            detail::bootloader::enableSWtrigger();
            break;
        }

        core::reset::mcuReset();
    }

    namespace detail
    {
        void errorHandler()
        {
            while (true)
            {
            }
        }
    }    // namespace detail
}    // namespace Board
