/*
    OpenDeck MIDI platform firmware
    Copyright (C) 2015-2018 Igor Petrovic
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

///
/// \brief Value indicating start of MIDI data when OpenDeck MIDI format is used.
///
#define OD_FORMAT_MIDI_DATA_START   0xF1

///
/// \brief Value indicating start of internal data when OpenDeck MIDI format is used.
/// Used to transfer internal data between main MCU and USB link.
///
#define OD_FORMAT_INT_DATA_START    0xF2

///
/// \brief List of all possible internal commands used in OpenDeck MIDI format.
///
typedef enum
{
    cmdFwUpdated,               ///< Signal to USB link MCU that the firmware has been updated on main MCU.
    cmdFwNotUpdated,            ///< Signal to USB link MCU that the firmware hasn't been updated on main MCU.
    cmdBtldrReboot,             ///< Signal to USB link MCU to reboot to bootloader mode.
    cmdUsbStateConnected,       ///< Signal to target MCU that USB link is connected to host.
    cmdUsbStateNotConnected,    ///< Signal to target MCU that USB link isn't connected to host.
    OD_FORMAT_COMMANDS
} odFormatCMD_t;