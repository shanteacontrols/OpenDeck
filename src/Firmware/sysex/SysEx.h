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

#ifndef SYSEX_H_
#define SYSEX_H_

#include <avr/io.h>
#include <stdio.h>
#include "Status.h"
#include "SpecialRequests.h"
#include "Config.h"
#include "../midi/MIDI.h"

typedef struct {

    uint8_t byte1;
    uint8_t byte2;
    uint8_t byte3;

} sysExManufacturerID;

#if PARAM_SIZE == 2
typedef int16_t sysExParameter_t;
#elif PARAM_SIZE == 1
typedef int8_t sysExParameter_t;
#else
#error Incorrect parameter size for SysEx
#endif

typedef struct {

    sysExParameter_t numberOfParameters;
    sysExParameter_t minValue;
    sysExParameter_t maxValue;
    uint8_t parts;
    bool newValueIgnored;

} sysExSection;

//class

class SysEx {

    public:
    SysEx();
    void handleSysEx(uint8_t *sysExArray, uint8_t size);
    void disableConf();
    void enableConf();
    bool configurationEnabled();
    bool addCustomRequest(uint8_t value);
    void addToResponse(sysExParameter_t value);

    void setHandleGet(sysExParameter_t(*fptr)(uint8_t block, uint8_t section, uint16_t index));
    void setHandleSet(bool(*fptr)(uint8_t block, uint8_t section, uint16_t index, sysExParameter_t newValue));
    void setHandleCustomRequest(bool(*fptr)(uint8_t value));

    bool addBlock(uint8_t sections);
    bool addSection(uint8_t block, sysExParameter_t numberOfParameters, sysExParameter_t minValue, sysExParameter_t maxValue);

    bool checkRequest();
    bool checkParameters();

    void sendCustomMessage(uint8_t id, sysExParameter_t value);

    private:

    typedef enum {

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

    typedef enum {

        indexByte = REQUEST_SIZE,
        newValueByte_single = indexByte+sizeof(sysExParameter_t),
        newValueByte_all = indexByte

    } sysExParameterByteOrder;

    typedef enum {

        //message wish
        sysExWish_get,
        sysExWish_set,
        sysExWish_backup,
        SYSEX_WISH_MAX

    } sysExWish;

    typedef enum {

        //wanted data amount
        sysExAmount_single,
        sysExAmount_all,
        SYSEX_AMOUNT_MAX

    } sysExAmount;

    typedef struct {

        //a struct containing entire info for block/message type

        uint8_t numberOfSections;
        uint8_t sectionCounter;
        sysExSection section[MAX_NUMBER_OF_SECTIONS];

    } sysExBlock;

    typedef struct {

        sysExManufacturerID id;
        sysExStatus_t status;
        sysExWish wish;
        sysExAmount amount;
        uint8_t block;
        uint8_t section;
        uint8_t part;
        sysExParameter_t index;
        sysExParameter_t newValue;

    } decodedMessage_t;

    bool checkID();
    bool checkSpecialRequests();
    bool checkWish();
    bool checkAmount();
    bool checkBlock();
    bool checkSection();
    bool checkPart();
    bool checkParameterIndex();
    bool checkNewValue();

    uint8_t generateMinMessageLenght();
    void setStatus(sysExStatus_t status);

    sysExParameter_t (*sendGetCallback)(uint8_t block, uint8_t section, uint16_t index);
    bool (*sendSetCallback)(uint8_t block, uint8_t section, uint16_t index, sysExParameter_t newValue);
    bool (*sendCustomRequestCallback)(uint8_t value);

    bool                sysExEnabled,
                        forcedSend;
    sysExBlock          sysExMessage[MAX_NUMBER_OF_BLOCKS];
    decodedMessage_t    decodedMessage;
    uint8_t             *sysExArray,
                        sysExArraySize,
                        customRequests[MAX_CUSTOM_REQUESTS],
                        customRequestCounter,
                        sysExBlockCounter,
                        responseSize;

};

extern SysEx sysEx;

#endif