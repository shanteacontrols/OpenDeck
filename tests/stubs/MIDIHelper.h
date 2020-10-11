#pragma once

#include <cinttypes>
#include <vector>
#include "midi/src/MIDI.h"

class MIDIHelper
{
    public:
    MIDIHelper() = default;

    static void sysExToUSBMIDIPacket(std::vector<uint8_t>& sysEx, std::vector<MIDI::USBMIDIpacket_t>& usbPacket)
    {
        usbPacket.clear();

        size_t inLength   = sysEx.size();
        size_t startIndex = 0;

        while (inLength > 3)
        {
            usbPacket.push_back({ MIDI::USBMIDIEvent(0, static_cast<uint8_t>(MIDI::usbMIDIsystemCin_t::sysExStartCin)), sysEx[startIndex + 0], sysEx[startIndex + 1], sysEx[startIndex + 2] });
            startIndex += 3;
            inLength -= 3;
        }

        if (inLength == 3)
        {
            usbPacket.push_back({ MIDI::USBMIDIEvent(0, static_cast<uint8_t>(MIDI::usbMIDIsystemCin_t::sysExStop3byteCin)), sysEx[startIndex + 0], sysEx[startIndex + 1], sysEx[startIndex + 2] });
        }
        else if (inLength == 2)
        {
            usbPacket.push_back({ MIDI::USBMIDIEvent(0, static_cast<uint8_t>(MIDI::usbMIDIsystemCin_t::sysExStop2byteCin)), sysEx[startIndex + 0], sysEx[startIndex + 1], 0 });
        }
        else if (inLength == 1)
        {
            usbPacket.push_back({ MIDI::USBMIDIEvent(0, static_cast<uint8_t>(MIDI::usbMIDIsystemCin_t::sysExStop1byteCin)), sysEx[startIndex + 0], 0, 0 });
        }
    }

    static bool parseUSBSysEx(std::vector<MIDI::USBMIDIpacket_t>& usbPacket, std::vector<uint8_t>& sysEx)
    {
        if (!usbPacket.size())
            return false;

        sysEx.clear();

        while (usbPacket.size())
        {
            uint8_t midiMessage = usbPacket.at(0).Event << 4;

            switch (midiMessage)
            {
            //1 byte messages
            case static_cast<uint8_t>(MIDI::usbMIDIsystemCin_t::sysCommon1byteCin):
            {
                //end of sysex
                sysEx.push_back(usbPacket.at(0).Data1);
                return true;
            }
            break;

            //sysex
            case static_cast<uint8_t>(MIDI::usbMIDIsystemCin_t::sysExStartCin):
            {
                //the message can be any length between 3 and MIDI_SYSEX_ARRAY_SIZE
                if (usbPacket.at(0).Data1 == 0xF0)
                    sysEx.clear();

                sysEx.push_back(usbPacket.at(0).Data1);
                sysEx.push_back(usbPacket.at(0).Data2);
                sysEx.push_back(usbPacket.at(0).Data3);
            }
            break;

            case static_cast<uint8_t>(MIDI::usbMIDIsystemCin_t::sysExStop2byteCin):
            {
                sysEx.push_back(usbPacket.at(0).Data1);
                sysEx.push_back(usbPacket.at(0).Data2);
                return true;
            }
            break;

            case static_cast<uint8_t>(MIDI::usbMIDIsystemCin_t::sysExStop3byteCin):
            {
                if (usbPacket.at(0).Data1 == 0xF0)
                    sysEx.clear();    //sysex message with 1 byte of payload

                sysEx.push_back(usbPacket.at(0).Data1);
                sysEx.push_back(usbPacket.at(0).Data2);
                sysEx.push_back(usbPacket.at(0).Data3);
                return true;
            }
            break;

            default:
                break;
            }

            usbPacket.erase(usbPacket.begin());
        }

        return false;
    }
};