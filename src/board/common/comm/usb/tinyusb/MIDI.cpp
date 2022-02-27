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

#include "midi/src/MIDI.h"
#include "board/Board.h"
#include "tusb.h"
#include <comm/usb/TinyUSB.h>

namespace Board
{
    namespace USB
    {
        bool readMIDI(midiPacket_t& packet)
        {
            tud_task();

            if (!tud_midi_available())
            {
                return false;
            }

            tud_midi_packet_read(&packet[0]);

            return true;
        }

        bool writeMIDI(midiPacket_t& packet)
        {
            if (!tud_midi_packet_write(&packet[0]))
            {
                return false;
            }

            tud_task();

            return true;
        }
    }    // namespace USB
}    // namespace Board