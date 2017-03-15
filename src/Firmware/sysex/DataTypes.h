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

#include "Config.h"

///
/// \brief Structure holding data for single SysEx section within block.
///
typedef struct
{
    sysExParameter_t numberOfParameters;
    sysExParameter_t minValue;
    sysExParameter_t maxValue;
    uint8_t parts;
    bool newValueIgnored;
} sysExSection;

///
/// \brief Structure holding data for single SysEx block.
///
typedef struct
{
    uint8_t numberOfSections;
    uint8_t sectionCounter;
    sysExSection section[MAX_NUMBER_OF_SECTIONS];
} sysExBlock;

///
/// \brief Descriptive list of bytes in SysEx message.
///
typedef enum
{
    startByte,      //0
    idByte_1,       //1
    idByte_2,       //2
    idByte_3,       //3
    statusByte,     //4
    partByte,       //5
    wishByte,       //6
    amountByte,     //7
    blockByte,      //8
    sectionByte,    //9
    REQUEST_SIZE,
    RESPONSE_SIZE = partByte + 1,
    MIN_MESSAGE_LENGTH = wishByte + 1 + 1,  //add next byte and end
    ML_REQ_STANDARD = REQUEST_SIZE + 1      //add end byte
} sysExRequestByteOrder;

///
/// \brief Byte order for parameters in SysEx message.
///
typedef enum
{
    indexByte = REQUEST_SIZE,
    newValueByte_single = indexByte+sizeof(sysExParameter_t),
    newValueByte_all = indexByte
} sysExParameterByteOrder;

///
/// \brief Descriptive list of SysEx wish bytes.
///
typedef enum
{
    sysExWish_get,
    sysExWish_set,
    sysExWish_backup,
    SYSEX_WISH_MAX
} sysExWish;

///
/// \brief Descriptive list of SysEx amount bytes.
///
typedef enum
{
    sysExAmount_single,
    sysExAmount_all,
    SYSEX_AMOUNT_MAX
} sysExAmount;

///
/// \brief Descriptive list of possible SysEx message statuses.
///
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

///
/// \brief List of special SysEx IDs.
///
typedef enum
{
    SYSEX_CLOSE_REQUEST,        //00
    HANDSHAKE_REQUEST,          //01
    BYTES_PER_VALUE_REQUEST,    //02
    PARAMS_PER_MESSAGE_REQUEST, //03
    SPECIAL_PARAMETERS
} sysEx_specialRequestID;

///
/// \brief Structure holding decoded request data.
///
typedef struct
{
    sysExStatus_t status;
    sysExWish wish;
    sysExAmount amount;
    uint8_t block;
    uint8_t section;
    uint8_t part;
    sysExParameter_t index;
    sysExParameter_t newValue;
} decodedMessage_t;
