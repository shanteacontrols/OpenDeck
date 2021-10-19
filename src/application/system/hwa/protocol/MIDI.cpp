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

#include "system/System.h"

bool System::HWAMIDI::init(MIDI::interface_t interface)
{
    if (interface == MIDI::interface_t::usb)
        return _system._hwa.protocol().midi().init(interface);

    // DIN

    auto mergeType    = static_cast<System::midiMergeType_t>(_system._database.read(Database::Section::global_t::midiMerge, System::midiMerge_t::mergeType));
    bool mergeEnabled = _system._database.read(Database::Section::global_t::midiFeatures, System::midiFeature_t::mergeEnabled);

    bool loopback = mergeType == System::midiMergeType_t::DINtoDIN && mergeEnabled;

    if (_dinMIDIenabled && (loopback == _dinMIDIloopbackEnabled))
        return true;    // nothing do do

    if (!_dinMIDIenabled)
    {
        if (_system._hwa.protocol().midi().init(interface))
        {
            _system._hwa.protocol().midi().setDINLoopback(loopback);

            _dinMIDIenabled         = true;
            _dinMIDIloopbackEnabled = loopback;

            return true;
        }
    }
    else
    {
        if (loopback != _dinMIDIloopbackEnabled)
        {
            // only the loopback parameter has changed
            _dinMIDIloopbackEnabled = loopback;
            _system._hwa.protocol().midi().setDINLoopback(loopback);
            return true;
        }
    }

    return false;
}

bool System::HWAMIDI::deInit(MIDI::interface_t interface)
{
    if (interface == MIDI::interface_t::din)
    {
        if (!_dinMIDIenabled)
            return true;    // nothing to do

        if (_system._hwa.protocol().midi().deInit(interface))
        {
            _dinMIDIenabled         = false;
            _dinMIDIloopbackEnabled = false;
            return true;
        }
    }
    else
    {
        return _system._hwa.protocol().midi().deInit(interface);
    }

    return false;
}

bool System::HWAMIDI::dinRead(uint8_t& value)
{
    return _system._hwa.protocol().midi().dinRead(value);
}

bool System::HWAMIDI::dinWrite(uint8_t value)
{
    return _system._hwa.protocol().midi().dinWrite(value);
}

bool System::HWAMIDI::usbRead(MIDI::USBMIDIpacket_t& USBMIDIpacket)
{
    return _system._hwa.protocol().midi().usbRead(USBMIDIpacket);
}

bool System::HWAMIDI::usbWrite(MIDI::USBMIDIpacket_t& USBMIDIpacket)
{
    return _system._hwa.protocol().midi().usbWrite(USBMIDIpacket);
}