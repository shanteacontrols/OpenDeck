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

#include <avr/eeprom.h>
#include "board/Board.h"
#include "board/Internal.h"
#include "board/common/constants/Reboot.h"
#include "board/common/io/Helpers.h"
#include "midi/src/Constants.h"
#include "core/src/arch/avr/Reset.h"
#include "core/src/general/ADC.h"
#include "core/src/arch/avr/Misc.h"
#include "core/src/general/IO.h"
#include "core/src/general/Helpers.h"
#include "core/src/general/Interrupt.h"
#include "common/OpenDeckMIDIformat/OpenDeckMIDIformat.h"

extern "C" void __cxa_pure_virtual()
{
    while (1)
        ;
}

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
    namespace
    {
        ///
        /// \brief Placeholder variable used only to reserve space in linker section.
        ///
        const uint32_t appLength __attribute__((section(".applen"))) __attribute__((used)) = 0;
    }    // namespace

    void init()
    {
        DISABLE_INTERRUPTS();

        //disable watchdog
        MCUSR &= ~(1 << WDRF);
        wdt_disable();

        detail::setup::io();

#ifndef BOARD_A_xu2
        detail::setup::adc();
#else
        UART::init(UART_USB_LINK_CHANNEL, UART_BAUDRATE_MIDI_OD);
#endif

#ifdef USB_MIDI_SUPPORTED
        detail::setup::usb();
#endif

        detail::setup::timers();
        ENABLE_INTERRUPTS();
    }

    void reboot(rebootType_t type)
    {
        switch (type)
        {
        case rebootType_t::rebootApp:
            eeprom_write_byte(reinterpret_cast<uint8_t*>(REBOOT_VALUE_EEPROM_LOCATION), APP_REBOOT_VALUE);
#ifndef USB_MIDI_SUPPORTED
            //signal to usb link to reboot as well
            //no need to do this for bootloader reboot - the bootloader already sends btldrReboot command to USB link
            MIDI::USBMIDIpacket_t USBMIDIpacket;

            USBMIDIpacket.Event = static_cast<uint8_t>(OpenDeckMIDIformat::command_t::appReboot);
            USBMIDIpacket.Data1 = 0x00;
            USBMIDIpacket.Data2 = 0x00;
            USBMIDIpacket.Data3 = 0x00;

            OpenDeckMIDIformat::write(UART_USB_LINK_CHANNEL, USBMIDIpacket, OpenDeckMIDIformat::packetType_t::internalCommand);
            while (!Board::UART::isTxEmpty(UART_USB_LINK_CHANNEL))
                ;
#endif
            break;

        case rebootType_t::rebootBtldr:
            eeprom_write_byte(reinterpret_cast<uint8_t*>(REBOOT_VALUE_EEPROM_LOCATION), BTLDR_REBOOT_VALUE);
            break;
        }

        core::reset::mcuReset();
    }

#ifndef BOARD_A_xu2
    bool checkNewRevision()
    {
        uint16_t crc_eeprom = eeprom_read_word(reinterpret_cast<uint16_t*>(SW_CRC_LOCATION_EEPROM));
#if (FLASHEND > 0xFFFF)
        uint32_t lastAddress = pgm_read_dword_far(core::misc::pgmGetFarAddress(APP_LENGTH_LOCATION));
        uint16_t crc_flash = pgm_read_word_far(core::misc::pgmGetFarAddress(lastAddress));
#else
        uint32_t lastAddress = pgm_read_dword(APP_LENGTH_LOCATION);
        uint16_t crc_flash = pgm_read_word(lastAddress);
#endif

        if (crc_eeprom != crc_flash)
        {
            eeprom_update_word(reinterpret_cast<uint16_t*>(SW_CRC_LOCATION_EEPROM), crc_flash);
            return true;
        }

        return false;
    }

    namespace eeprom
    {
        bool read(uint32_t address, LESSDB::sectionParameterType_t type, int32_t& value)
        {
            switch (type)
            {
            case LESSDB::sectionParameterType_t::bit:
            case LESSDB::sectionParameterType_t::byte:
            case LESSDB::sectionParameterType_t::halfByte:
                value = eeprom_read_byte(reinterpret_cast<uint8_t*>(address));
                break;

            case LESSDB::sectionParameterType_t::word:
                value = eeprom_read_word(reinterpret_cast<uint16_t*>(address));
                break;

            default:
                // case LESSDB::sectionParameterType_t::dword:
                value = eeprom_read_dword(reinterpret_cast<uint32_t*>(address));
                break;
            }

            return true;
        }

        bool write(uint32_t address, int32_t value, LESSDB::sectionParameterType_t type)
        {
            switch (type)
            {
            case LESSDB::sectionParameterType_t::bit:
            case LESSDB::sectionParameterType_t::byte:
            case LESSDB::sectionParameterType_t::halfByte:
                eeprom_update_byte(reinterpret_cast<uint8_t*>(address), value);
                break;

            case LESSDB::sectionParameterType_t::word:
                eeprom_update_word(reinterpret_cast<uint16_t*>(address), value);
                break;

            default:
                // case LESSDB::sectionParameterType_t::dword:
                eeprom_update_dword(reinterpret_cast<uint32_t*>(address), value);
                break;
            }

            return true;
        }
    }    // namespace eeprom
#endif
}    // namespace Board

#ifdef ADC
///
/// \brief ADC ISR used to read values from multiplexers.
///
ISR(ADC_vect)
{
    Board::detail::io::adcISRHandler(ADC);
}
#endif

///
/// \brief Main interrupt service routine.
/// Used to control I/O on board and to update current run time.
///
ISR(TIMER0_COMPA_vect)
{
    static bool _1ms = true;
    _1ms = !_1ms;

    if (_1ms)
    {
        core::timing::detail::rTime_ms++;

#ifdef LEDS_SUPPORTED
        Board::detail::io::checkDigitalOutputs();
#endif
#ifdef LED_INDICATORS
        Board::detail::io::checkIndicators();
#endif
    }

#ifndef BOARD_A_xu2
    Board::detail::io::checkDigitalInputs();
#endif
}