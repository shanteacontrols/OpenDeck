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

#pragma once

typedef enum
{
    REQUEST,                //0x00
    ACK,                    //0x01
    ERROR_STATUS,           //0x02
    ERROR_HANDSHAKE,        //0x03
    ERROR_WISH,             //0x04
    ERROR_AMOUNT,           //0x05
    ERROR_BLOCK,            //0x06
    ERROR_SECTION,          //0x07
    ERROR_PART,             //0x08
    ERROR_INDEX,            //0x09
    ERROR_NEW_VALUE,        //0x0A
    ERROR_MESSAGE_LENGTH,   //0x0B
    ERROR_WRITE             //0x0C
} sysExStatus_t;
