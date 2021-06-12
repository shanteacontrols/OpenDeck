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

#include "SysExParser.h"

bool SysExParser::isValidMessage(MIDI::USBMIDIpacket_t& packet)
{
    if (parse(packet))
        return verify();

    return false;
}

bool SysExParser::parse(MIDI::USBMIDIpacket_t& packet)
{
    //MIDIEvent.Event is CIN, see midi10.pdf
    //shift cin four bytes left to get message type
    uint8_t midiMessage = packet.Event << 4;

    switch (midiMessage)
    {
    case static_cast<uint8_t>(usbMIDIsystemCin_t::sysCommon1byteCin):
    case static_cast<uint8_t>(usbMIDIsystemCin_t::singleByte):
        if (packet.Data1 == 0xF7)
        {
            //end of sysex
            _sysExArray[_sysExArrayLength] = packet.Data1;
            _sysExArrayLength++;
            return true;
        }
        break;

    case static_cast<uint8_t>(usbMIDIsystemCin_t::sysExStartCin):
        if (packet.Data1 == 0xF0)
            _sysExArrayLength = 0;    //this is a new sysex message, reset length

        _sysExArray[_sysExArrayLength] = packet.Data1;
        _sysExArrayLength++;
        _sysExArray[_sysExArrayLength] = packet.Data2;
        _sysExArrayLength++;
        _sysExArray[_sysExArrayLength] = packet.Data3;
        _sysExArrayLength++;
        return false;
        break;

    case static_cast<uint8_t>(usbMIDIsystemCin_t::sysExStop2byteCin):
        _sysExArray[_sysExArrayLength] = packet.Data1;
        _sysExArrayLength++;
        _sysExArray[_sysExArrayLength] = packet.Data2;
        _sysExArrayLength++;
        return true;
        break;

    case static_cast<uint8_t>(usbMIDIsystemCin_t::sysExStop3byteCin):
        if (packet.Data1 == 0xF0)
            _sysExArrayLength = 0;    //sysex message with 1 byte of payload

        _sysExArray[_sysExArrayLength] = packet.Data1;
        _sysExArrayLength++;
        _sysExArray[_sysExArrayLength] = packet.Data2;
        _sysExArrayLength++;
        _sysExArray[_sysExArrayLength] = packet.Data3;
        _sysExArrayLength++;
        return true;
        break;

    default:
        return false;
        break;
    }

    return false;
}

size_t SysExParser::dataBytes()
{
    if (!verify())
        return 0;

    return (_sysExArrayLength - 2 - 3) / 2;
}

bool SysExParser::value(size_t index, uint8_t& data)
{
    size_t arrayIndex = DATA_START_BYTE + index * 2;

    if ((arrayIndex + 1) >= _sysExArrayLength)
        return false;

    uint16_t data16;

    mergeTo14bit(data16, _sysExArray[arrayIndex], _sysExArray[arrayIndex + 1]);

    data = data16 & 0xFF;

    return true;
}

bool SysExParser::verify()
{
    if (_sysExArray[1] != SYSEX_MANUFACTURER_ID_0)
        return false;

    if (_sysExArray[2] != SYSEX_MANUFACTURER_ID_1)
        return false;

    if (_sysExArray[3] != SYSEX_MANUFACTURER_ID_2)
        return false;

    //firmware sysex message should contain at least:
    //start byte
    //three ID bytes
    //two data bytes
    //stop byte
    if (_sysExArrayLength < 7)
        return false;

    return true;
}

/// Convert 7-bit high and low bytes to single 14-bit value.
/// param [in,out]: value   Resulting 14-bit value.
/// param [in,out]: high    Higher 7 bits.
/// param [in,out]: low     Lower 7 bits.
void SysExParser::mergeTo14bit(uint16_t& value, uint8_t high, uint8_t low)
{
    if (high & 0x01)
        low |= (1 << 7);
    else
        low &= ~(1 << 7);

    high >>= 1;

    uint16_t joined;

    joined = high;
    joined <<= 8;
    joined |= low;

    value = joined;
}