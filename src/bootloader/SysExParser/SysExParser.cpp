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

#include "SysExParser.h"
#include "util/conversion/Conversion.h"
#include "system/Config.h"

bool SysExParser::isValidMessage(MIDI::usbMIDIPacket_t& packet)
{
    if (parse(packet))
    {
        return verify();
    }

    return false;
}

bool SysExParser::parse(MIDI::usbMIDIPacket_t& packet)
{
    // see midi10.pdf
    // shift cin four bytes left to get message type

    uint8_t midiMessage = packet[MIDI::USB_EVENT] << 4;

    switch (midiMessage)
    {
    case static_cast<uint8_t>(usbMIDIsystemCin_t::SYS_COMMON1BYTE_CIN):
    case static_cast<uint8_t>(usbMIDIsystemCin_t::SINGLE_BYTE):
    {
        if (packet[MIDI::USB_DATA1] == 0xF7)
        {
            // end of sysex
            _sysExArray[_sysExArrayLength] = packet[MIDI::USB_DATA1];
            _sysExArrayLength++;
            return true;
        }
    }
    break;

    case static_cast<uint8_t>(usbMIDIsystemCin_t::SYS_EX_START_CIN):
    {
        if (packet[MIDI::USB_DATA1] == 0xF0)
        {
            _sysExArrayLength = 0;    // this is a new sysex message, reset length
        }

        _sysExArray[_sysExArrayLength] = packet[MIDI::USB_DATA1];
        _sysExArrayLength++;
        _sysExArray[_sysExArrayLength] = packet[MIDI::USB_DATA2];
        _sysExArrayLength++;
        _sysExArray[_sysExArrayLength] = packet[MIDI::USB_DATA3];
        _sysExArrayLength++;
        return false;
    }
    break;

    case static_cast<uint8_t>(usbMIDIsystemCin_t::SYS_EX_STOP2BYTE_CIN):
    {
        _sysExArray[_sysExArrayLength] = packet[MIDI::USB_DATA1];
        _sysExArrayLength++;
        _sysExArray[_sysExArrayLength] = packet[MIDI::USB_DATA2];
        _sysExArrayLength++;
        return true;
    }
    break;

    case static_cast<uint8_t>(usbMIDIsystemCin_t::SYS_EX_STOP3BYTE_CIN):
    {
        if (packet[MIDI::USB_DATA1] == 0xF0)
        {
            _sysExArrayLength = 0;    // sysex message with 1 byte of payload
        }

        _sysExArray[_sysExArrayLength] = packet[MIDI::USB_DATA1];
        _sysExArrayLength++;
        _sysExArray[_sysExArrayLength] = packet[MIDI::USB_DATA2];
        _sysExArrayLength++;
        _sysExArray[_sysExArrayLength] = packet[MIDI::USB_DATA3];
        _sysExArrayLength++;
        return true;
    }
    break;

    default:
        return false;
    }

    return false;
}

size_t SysExParser::dataBytes()
{
    if (!verify())
    {
        return 0;
    }

    return (_sysExArrayLength - 2 - 3) / 2;
}

bool SysExParser::value(size_t index, uint8_t& data)
{
    size_t arrayIndex = DATA_START_BYTE + index * 2;

    if ((arrayIndex + 1) >= _sysExArrayLength)
    {
        return false;
    }

    auto merged = Util::Conversion::Merge14bit(_sysExArray[arrayIndex], _sysExArray[arrayIndex + 1]);
    data        = merged.value() & 0xFF;

    return true;
}

bool SysExParser::verify()
{
    using namespace System;

    if (_sysExArray[1] != Config::SYSEX_MANUFACTURER_ID_0)
    {
        return false;
    }

    if (_sysExArray[2] != Config::SYSEX_MANUFACTURER_ID_1)
    {
        return false;
    }

    if (_sysExArray[3] != Config::SYSEX_MANUFACTURER_ID_2)
    {
        return false;
    }

    // firmware sysex message should contain at least:
    // start byte
    // three ID bytes
    // two data bytes
    // stop byte
    if (_sysExArrayLength < 7)
    {
        return false;
    }

    return true;
}