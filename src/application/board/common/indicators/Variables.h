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
/// \brief Variables indicating whether MIDI in/out LED indicators should be turned on.
/// External definition, must be implemented for specific board in order to use it.
/// State of these variables is set to true externally when MIDI event happens.
/// Handling should check if their state is true, and if it is, LED indicator should
/// be turned on, variable state should be set to false and timeout countdown should be started.
/// @{

extern volatile bool        MIDIreceived;
extern volatile bool        MIDIsent;

/// @}
