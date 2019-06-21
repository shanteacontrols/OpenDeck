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

#pragma once

#include <inttypes.h>

///
/// \brief List of all possible internal commands used in OpenDeck MIDI format.
///
enum class odFormatCMD_t : uint8_t
{
    cmdFwUpdated,       ///< Signal to USB link MCU that the firmware has been updated on main MCU.
    cmdFwNotUpdated,    ///< Signal to USB link MCU that the firmware hasn't been updated on main MCU.
    cmdBtldrReboot      ///< Signal to USB link MCU to reboot to bootloader mode.
};

enum class odPacketType_t : uint8_t
{
    packetInvalid = 0,
    packetMIDI = 0xF1,        ///< MIDI packet in OpenDeck format.
                              ///< Indicates start of MIDI data when OpenDeck MIDI format is used.
    packetIntCMD = 0xF2,      ///< Internal command used for target MCU <> USB link communication.
                              ///< Indicates start of internal data when OpenDeck MIDI format is used.
    packetMIDI_dc_m = 0xF3    ///< MIDI packet sent from master to slaves in daisy chain configuration.
                              ///< Indicates that origin of MIDI message is OpenDeck master in daisy-chain configuration.
                              ///< Since all the messages are forwarded between slaves, avoid routing message from master
                              ///< back to USB master (filter out).
};