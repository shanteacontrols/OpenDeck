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

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include "board/Board.h"
#include "board/Internal.h"
#include "core/src/general/Helpers.h"
#include "core/src/arch/avr/Misc.h"
#include "core/src/general/Interrupt.h"
#include "core/src/general/Timing.h"
#include "common/OpenDeckMIDIformat/OpenDeckMIDIformat.h"

/// Location at which reboot type is written in EEPROM when initiating software reset.
#define REBOOT_VALUE_EEPROM_LOCATION E2END

namespace Board
{
    namespace detail
    {
        namespace bootloader
        {
            fwType_t btldrTriggerSoftType()
            {
                if (eeprom_read_byte((uint8_t*)REBOOT_VALUE_EEPROM_LOCATION) == static_cast<uint8_t>(fwType_t::bootloader))
                    return fwType_t::bootloader;
                else
                    return fwType_t::application;
            }

            void setSWtrigger(fwType_t btldrTriggerSoftType)
            {
                if (btldrTriggerSoftType == fwType_t::cdc)
                    btldrTriggerSoftType = fwType_t::application;

                eeprom_write_byte((uint8_t*)REBOOT_VALUE_EEPROM_LOCATION, static_cast<uint8_t>(btldrTriggerSoftType));
            }

            void runApplication()
            {
                __asm__ __volatile__(
                    // Jump to RST vector
                    "clr r30\n"
                    "clr r31\n"
                    "ijmp\n");
            }

            void runBootloader()
            {
                //relocate the interrupt vector table to the bootloader section
                MCUCR = (1 << IVCE);
                MCUCR = (1 << IVSEL);

                ENABLE_INTERRUPTS();

                detail::bootloader::indicate();

#if defined(USB_LINK_MCU) || !defined(USB_MIDI_SUPPORTED)
                Board::UART::init(UART_CHANNEL_USB_LINK, UART_BAUDRATE_MIDI_OD);

#ifndef USB_MIDI_SUPPORTED
                // make sure USB link goes to bootloader mode as well
                MIDI::USBMIDIpacket_t packet;

                packet.Event = static_cast<uint8_t>(OpenDeckMIDIformat::command_t::btldrReboot);
                packet.Data1 = 0x00;
                packet.Data2 = 0x00;
                packet.Data3 = 0x00;

                //add some delay - it case of hardware btldr entry both MCUs will boot up in the same time
                //so it's possible USB link MCU will miss this packet

                core::timing::waitMs(1000);

                OpenDeckMIDIformat::write(UART_CHANNEL_USB_LINK, packet, OpenDeckMIDIformat::packetType_t::internalCommand);
#endif
#endif

#ifdef USB_MIDI_SUPPORTED
                detail::setup::usb();
#endif
            }
        }    // namespace bootloader
    }        // namespace detail
}    // namespace Board