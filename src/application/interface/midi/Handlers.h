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

#include "board/Board.h"
#include "Constants.h"

///
/// \brief Configures UART read/write handlers for MIDI module.
/// @param [in] channel     UART channel on MCU.
///
void setupMIDIoverUART(uint8_t channel);

///
/// \brief Configures OpenDeck UART MIDI format on specified UART channel.
/// OpenDeck UART MIDI format is a specially formatted MIDI message which
/// is sent over UART and uses USB MIDI packet format with byte value
/// OD_FORMAT_MIDI_DATA_START prepended before the message start and XOR of
/// 4 USB packet bytes as the last byte. XOR byte is used for error checking.
/// This is used to avoid parsing of MIDI messages when using boards such as Arduino
/// Mega or Arduino Uno which feature two separate MCUs - the main one and the other for
/// USB communication. Using this format USB MCU can quickly forward the received message
/// from UART to USB interface - first byte (OD_FORMAT_MIDI_DATA_START) and last byte
/// (XOR value) are removed from the output.
/// @param [in] channel     UART channel on MCU.
///
void setupMIDIoverUART_OD(uint8_t channel);

///
/// \brief Used to read MIDI data from USB interface and to immediately transfer received data
/// to UART interface in OpenDeck format.
/// @param [in] USBMIDIpacket   Pointer to structure holding MIDI data to write.
/// \returns True if data is available, false otherwise.
///
bool usbMIDItoUART_OD(USBMIDIpacket_t& USBMIDIpacket);

///
/// \brief Used to read MIDI data using custom OpenDeck format from UART interface.
/// @param [in] channel     UART channel on MCU.
/// \returns True if data is available, false otherwise.
///
bool uartReadMIDI_OD(uint8_t channel);

///
/// \brief Used to write MIDI data using custom OpenDeck format to UART interface.
/// @param [in] channel     UART channel on MCU.
/// @param [in] USBMIDIpacket   Pointer to structure holding MIDI data to write.
/// \returns True on success, false otherwise.
///
bool uartWriteMIDI_OD(uint8_t channel, USBMIDIpacket_t& USBMIDIpacket);