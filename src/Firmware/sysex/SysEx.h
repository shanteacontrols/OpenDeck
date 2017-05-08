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
#include "../midi/src/MIDI.h"
#include "DataTypes.h"

///
/// \brief Configuration protocol created using custom SysEx MIDI messages.
/// \ingroup conf
/// @{
///
class SysEx
{
    public:
    SysEx();
    void handleMessage(uint8_t *sysExArray, uint8_t size);
    void decode();
    bool configurationEnabled();
    bool addCustomRequest(uint8_t value);
    void startResponse();
    void addToResponse(sysExParameter_t value);
    void sendResponse();

    void setHandleGet(sysExParameter_t(*fptr)(uint8_t block, uint8_t section, uint16_t index));
    void setHandleSet(bool(*fptr)(uint8_t block, uint8_t section, uint16_t index, sysExParameter_t newValue));
    void setHandleCustomRequest(bool(*fptr)(uint8_t value));

    bool addBlock();
    bool addBlocks(uint8_t numberOfBlocks);
    bool addSection(uint8_t blockID, sysExSection section);

    bool checkRequest();
    bool checkParameters();

    private:
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
/// @}