/*
    OpenDeck MIDI platform firmware
    Copyright (C) 2015-2017 Igor Petrovic

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

#define normalizeChannel(channel)   (((channel - 1) & MAX_MIDI_CHANNEL_MASK))
#define normalizeData(data)         (data & MAX_MIDI_VALUE_MASK)
#define lowByte_7bit(value)         ((value) & 0x7F)
#define highByte_7bit(value)        ((value >> 7) & 0x7f)