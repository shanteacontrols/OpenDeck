/*
    OpenDeck MIDI platform firmware
    Copyright (C) 2015, 2016 Igor Petrovic

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

#ifndef SPECIALREQUESTS_H_
#define SPECIALREQUESTS_H_

typedef enum {

    HANDSHAKE_REQUEST,          //00
    BYTES_PER_VALUE_REQUEST,    //01
    PARAMS_PER_MESSAGE_REQUEST, //02
    SPECIAL_PARAMETERS

} sysEx_specialRequestID;

#endif