/*

Copyright Igor Petrovic

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

#ifdef PROJECT_TARGET_SUPPORT_USB
#ifdef BOARD_USE_TINYUSB

#include "board/board.h"

#include "tusb.h"

namespace board::usb
{
    bool readMidi(lib::midi::usb::Packet& packet)
    {
        tud_task();

        if (!tud_midi_available())
        {
            return false;
        }

        tud_midi_packet_read(&packet.data[0]);

        return true;
    }

    bool writeMidi(lib::midi::usb::Packet& packet)
    {
        if (!tud_midi_packet_write(&packet.data[0]))
        {
            return false;
        }

        tud_task();

        return true;
    }
}    // namespace board::usb

#endif
#endif