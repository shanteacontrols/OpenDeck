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

#include "SysEx.h"

///
/// \brief Default constructor.
///
SysEx::SysEx()
{
    sendGetCallback             = NULL;
    sendSetCallback             = NULL;
    sendCustomRequestCallback   = NULL;

    sysExEnabled = false;
    forcedSend = false;

    for (int i=0; i<MAX_NUMBER_OF_BLOCKS; i++)
    {
        sysExMessage[i].numberOfSections = INVALID_VALUE;
        sysExMessage[i].sectionCounter = 0;

        for (int j=0; j<MAX_NUMBER_OF_SECTIONS; j++)
        {
            sysExMessage[i].section[j].numberOfParameters = INVALID_VALUE;
            sysExMessage[i].section[j].minValue = INVALID_VALUE;
            sysExMessage[i].section[j].maxValue = INVALID_VALUE;
        }
    }

    for (int i=0; i<MAX_CUSTOM_REQUESTS; i++)
        customRequests[i] = INVALID_VALUE;

    customRequestCounter = 0;
    sysExBlockCounter = 0;
}

///
/// \brief Checks whether the SysEx configuration is enabled or not.
/// \returns True if enabled, false otherwise.
///
bool SysEx::configurationEnabled()
{
    return sysExEnabled;
}

///
/// \brief Adds custom request.
/// If added byte is found in incoming message, and message is formatted as special request, custom message handler is called.
/// It is up to user to decide on action.
/// @param [in] value   Custom request value.
///
bool SysEx::addCustomRequest(uint8_t value)
{
    if (customRequestCounter >= MAX_CUSTOM_REQUESTS)
        return false;

    //don't add custom string if it's already defined as one of default strings
    if (value < SPECIAL_PARAMETERS)
        return false;

    customRequests[customRequestCounter] = value;
    customRequestCounter++;
    return true;
}

///
/// \brief Adds single SysEx block.
/// \returns True on success, false otherwise.
///
bool SysEx::addBlock()
{
    if (sysExBlockCounter >= MAX_NUMBER_OF_BLOCKS)
        return false;

    sysExBlockCounter++;
    return true;
}

///
/// \brief Adds specified number of SysEx blocks.
/// @param [in] numberOfBlocks  Number of blocks to add.
/// \returns True on success, false otherwise.
///
bool SysEx::addBlocks(uint8_t numberOfBlocks)
{
    if (sysExBlockCounter+numberOfBlocks >= MAX_NUMBER_OF_BLOCKS)
        return false;

    sysExBlockCounter += numberOfBlocks;
    return true;
}

///
/// \brief Adds section to specified block.
/// @param [in] blockID Block on which to add section.
/// @param [in] section Structure holding description of section.
/// \returns True on success, false otherwise.
///
bool SysEx::addSection(uint8_t blockID, sysExSection section)
{
    if (sysExMessage[blockID].sectionCounter >= MAX_NUMBER_OF_SECTIONS)
        return false;

    sysExMessage[blockID].section[sysExMessage[blockID].sectionCounter].numberOfParameters = section.numberOfParameters;
    sysExMessage[blockID].section[sysExMessage[blockID].sectionCounter].minValue = section.minValue;
    sysExMessage[blockID].section[sysExMessage[blockID].sectionCounter].maxValue = section.maxValue;

    //based on number of parameters, calculate how many parts message has in case of set/all request and get/all response
    sysExMessage[blockID].section[sysExMessage[blockID].sectionCounter].parts = sysExMessage[blockID].section[sysExMessage[blockID].sectionCounter].numberOfParameters / PARAMETERS_PER_MESSAGE;

    if (sysExMessage[blockID].section[sysExMessage[blockID].sectionCounter].numberOfParameters % PARAMETERS_PER_MESSAGE)
        sysExMessage[blockID].section[sysExMessage[blockID].sectionCounter].parts++;

    sysExMessage[blockID].sectionCounter++;
    return true;
}

///
/// \brief Handles incoming SysEx message.
/// @param [in] array   SysEx array.
/// @param [in] size    Array size.
///
void SysEx::handleMessage(uint8_t *array, uint8_t size)
{
    //save pointer to received array so we can manipulate it directly
    sysExArray = array;
    sysExArraySize = size;
    responseSize = RESPONSE_SIZE;

    if (sysExArraySize < MIN_MESSAGE_LENGTH)
        return; //ignore small messages

    decode();

    if (!forcedSend)
    {
        sysExArray[responseSize] = 0xF7;
        responseSize++;

        midi.sendSysEx(responseSize, sysExArray, true);
    }

    forcedSend = false;
}

///
/// \brief Decodes received message.
///
void SysEx::decode()
{
    decodedMessage.status = (sysExStatus_t)sysExArray[(uint8_t)statusByte];

    if (decodedMessage.status != REQUEST)
    {
        //don't let status be anything but request
        setStatus(ERROR_STATUS);
        return;
    }

    if (!checkID())
        return; //don't send response to wrong ID

    if (checkSpecialRequests())
        return; //special request was handled by now

    //message appears to be fine for now
    if (!sysExEnabled)
    {
        //message is fine, but handshake hasn't been received
        setStatus(ERROR_HANDSHAKE);
        return;
    }

    if (sysExArraySize < REQUEST_SIZE)
    {
        setStatus(ERROR_MESSAGE_LENGTH);
        return;
    }

    //don't try to request these parameters if the size is too small
    decodedMessage.part = sysExArray[(uint8_t)partByte];
    decodedMessage.wish = (sysExWish)sysExArray[(uint8_t)wishByte];
    decodedMessage.amount = (sysExAmount)sysExArray[(uint8_t)amountByte];
    decodedMessage.block = sysExArray[(uint8_t)blockByte];
    decodedMessage.section = sysExArray[(uint8_t)sectionByte];

    if (!checkRequest())
        return;

    uint16_t minLength = generateMinMessageLenght();

    if (sysExArraySize < minLength || !minLength)
    {
        setStatus(ERROR_MESSAGE_LENGTH);
        return;
    }

    if (checkParameters())
    {
        //message is ok
    }
}

///
/// \brief Checks whether the manufacturer ID in message is correct.
/// @returns    True if valid, false otherwise.
///
bool SysEx::checkID()
{
    if (sysExArraySize < (idByte_3+1))
        return false;

    return
    (
        (sysExArray[idByte_1] == SYS_EX_M_ID_0)    &&
        (sysExArray[idByte_2] == SYS_EX_M_ID_1)    &&
        (sysExArray[idByte_3] == SYS_EX_M_ID_2)
    );
}

///
/// \brief Checks whether the request belong to special group of requests or not
/// \returns    True if request is special, false otherwise.
///
bool SysEx::checkSpecialRequests()
{
    if (sysExArraySize != MIN_MESSAGE_LENGTH)
        return false;

    switch(sysExArray[wishByte])
    {
        case SYSEX_CLOSE_REQUEST:
        if (!sysExEnabled)
        {
            //connection can't be closed if it isn't opened
            setStatus(ERROR_HANDSHAKE);
            return true;
        }
        else
        {
            //close sysex connection
            sysExEnabled = false;
            setStatus(ACK);
            return true;
        }
        break;

        case HANDSHAKE_REQUEST:
        //hello message, necessary for allowing configuration
        sysExEnabled = true;
        setStatus(ACK);
        return true;

        case BYTES_PER_VALUE_REQUEST:
        if (sysExEnabled)
        {
            setStatus(ACK);
            addToResponse(PARAM_SIZE);
        }
        else
        {
            setStatus(ERROR_HANDSHAKE);
        }
        return true;

        case PARAMS_PER_MESSAGE_REQUEST:
        if (sysExEnabled)
        {
            setStatus(ACK);
            addToResponse(PARAMETERS_PER_MESSAGE);
        }
        else
        {
            setStatus(ERROR_HANDSHAKE);
        }
        return true;

        default:
        //check for custom string
        for (int i=0; i<MAX_CUSTOM_REQUESTS; i++)
        {
            if (customRequests[i] == INVALID_VALUE)
                continue;

            if (customRequests[i] != sysExArray[wishByte])
                continue;

            if (sysExEnabled)
            {
                setStatus(ACK);
                sendCustomRequestCallback(customRequests[i]);
            }
            else
            {
                setStatus(ERROR_HANDSHAKE);
            }

            return true;
        }
        //custom string not found
        setStatus(ERROR_WISH);
        return true;
    }
}

///
/// \brief Checks whether request is valid by calling other checking functions.
/// @returns    True if valid, false otherwise.
///
bool SysEx::checkRequest()
{
    if (!checkWish())
    {
        setStatus(ERROR_WISH);
        return false;
    }

    if (!checkBlock())
    {
        setStatus(ERROR_BLOCK);
        return false;
    }

    if (!checkSection())
    {
        setStatus(ERROR_SECTION);
        return false;
    }

    if (!checkAmount())
    {
        setStatus(ERROR_AMOUNT);
        return false;
    }

    if (!checkPart())
    {
        setStatus(ERROR_PART);
        return false;
    }

    return true;
}

///
/// \brief Checks all parameters in message, builds and sends response.
/// \returns    True if entire request is valid, false otherwise.
///
bool SysEx::checkParameters()
{
    //sysex request is fine
    //start building response
    setStatus(ACK);

    uint8_t loops = 1, responseSize_ = responseSize;
    uint16_t startIndex = 0, endIndex = 1;
    bool allParts = false;

    if (decodedMessage.amount == sysExAmount_single)
    {
        #if PARAM_SIZE == 2
        encDec_14bit decoded;
        //index
        decoded.high = sysExArray[indexByte];
        decoded.low = sysExArray[indexByte+1];
        decodedMessage.index = decoded.decode14bit();
        #elif PARAM_SIZE == 1
        decodedMessage.index = sysExArray[indexByte];
        #endif
        decodedMessage.index += (PARAMETERS_PER_MESSAGE*decodedMessage.part);

        if (decodedMessage.wish == sysExWish_set)
        {
            //new value
            #if PARAM_SIZE == 2
            decoded.high = sysExArray[newValueByte_single];
            decoded.low = sysExArray[newValueByte_single+1];
            decodedMessage.newValue = decoded.decode14bit();
            #elif PARAM_SIZE == 1
            decodedMessage.newValue = sysExArray[newValueByte_single];
            #endif
        }
    }

    if ((decodedMessage.wish == sysExWish_backup) || (decodedMessage.wish == sysExWish_get))
    {
        if (decodedMessage.part == 127)
        {
            loops = sysExMessage[decodedMessage.block].section[decodedMessage.section].parts;
            forcedSend = true;
            allParts = true;
        }

        if (decodedMessage.wish == sysExWish_backup)
        {
            //don't overwrite anything if backup is requested
            responseSize_ = sysExArraySize - 1;
            //convert response to request
            sysExArray[(uint8_t)statusByte] = REQUEST;
            //now convert wish to set
            sysExArray[(uint8_t)wishByte] = (uint8_t)sysExWish_set;
            //decoded message wish needs to be set to get so that we can retrieve parameters
            decodedMessage.wish = sysExWish_get;
        }
    }

    for (int j=0; j<loops; j++)
    {
        responseSize = responseSize_;

        if (forcedSend)
        {
            decodedMessage.part = j;
            sysExArray[partByte] = j;
        }

        if (decodedMessage.amount == sysExAmount_all)
        {
            startIndex = PARAMETERS_PER_MESSAGE*decodedMessage.part;
            endIndex = startIndex + PARAMETERS_PER_MESSAGE;

            if ((sysExParameter_t)endIndex > sysExMessage[decodedMessage.block].section[decodedMessage.section].numberOfParameters)
                endIndex = sysExMessage[decodedMessage.block].section[decodedMessage.section].numberOfParameters;
        }

        for (uint16_t i=startIndex; i<endIndex; i++)
        {
            switch(decodedMessage.wish)
            {
                case sysExWish_get:
                if (decodedMessage.amount == sysExAmount_single)
                {
                    if (!checkParameterIndex())
                    {
                        setStatus(ERROR_INDEX);
                        return false;
                    }
                    else
                    {
                        addToResponse(sendGetCallback(decodedMessage.block, decodedMessage.section, decodedMessage.index));
                    }
                }
                else
                {
                    //get all params - no index is specified
                    addToResponse(sendGetCallback(decodedMessage.block, decodedMessage.section, i));
                }
                break;

                case sysExWish_set:
                if (decodedMessage.amount == sysExAmount_single)
                {
                    if (!checkParameterIndex())
                    {
                        setStatus(ERROR_INDEX);
                        return false;
                    }

                    if (!checkNewValue())
                    {
                        setStatus(ERROR_NEW_VALUE);
                        return false;
                    }

                    if (sendSetCallback(decodedMessage.block, decodedMessage.section, decodedMessage.index, decodedMessage.newValue))
                        return true;
                }
                else
                {
                    uint8_t arrayIndex = (i-startIndex);

                    #if PARAM_SIZE == 2
                    arrayIndex *= sizeof(sysExParameter_t);
                    arrayIndex += newValueByte_all;
                    encDec_14bit decoded;
                    decoded.high = sysExArray[arrayIndex];
                    decoded.low = sysExArray[arrayIndex+1];
                    decodedMessage.newValue = decoded.decode14bit();
                    #elif PARAM_SIZE == 1
                    decodedMessage.newValue = sysExArray[arrayIndex+newValueByte_all];
                    #endif

                    if (!checkNewValue())
                    {
                        setStatus(ERROR_NEW_VALUE);
                        return false;
                    }

                    if (!sendSetCallback(decodedMessage.block, decodedMessage.section, i, decodedMessage.newValue))
                    {
                        setStatus(ERROR_WRITE);
                        return false;
                    }
                }
                break;

                default:
                setStatus(ERROR_WISH);
                return false;
            }
        }

        if (forcedSend)
        {
            sysExArray[responseSize] = 0xF7;
            responseSize++;

            midi.sendSysEx(responseSize, sysExArray, true);
        }
    }

    if (allParts)
    {
        //send ACK message at the end
        responseSize = 0;
        addToResponse(0xF0);
        addToResponse(SYS_EX_M_ID_0);
        addToResponse(SYS_EX_M_ID_1);
        addToResponse(SYS_EX_M_ID_2);
        addToResponse(ACK);
        addToResponse(0x7F);
        addToResponse(decodedMessage.wish);
        addToResponse(decodedMessage.amount);
        addToResponse(decodedMessage.block);
        addToResponse(decodedMessage.section);
        addToResponse(0xF7);

        midi.sendSysEx(responseSize, sysExArray, true);
    }

    return true;
}

///
/// \brief Generates minimum message length based on other parameters in message.
/// \returns    Message length in bytes.
///
uint8_t SysEx::generateMinMessageLenght()
{
    uint16_t size = 0;

    switch(decodedMessage.amount)
    {
        case sysExAmount_single:
        switch(decodedMessage.wish)
        {
            case sysExWish_get:
            case sysExWish_backup:
            return ML_REQ_STANDARD + sizeof(sysExParameter_t); //add parameter length
            break;

            case sysExWish_set:
            return ML_REQ_STANDARD + 2*sizeof(sysExParameter_t); //add parameter length and new value length
            break;

            default:
            return 0;
        }
        break;

        case sysExAmount_all:
        switch(decodedMessage.wish)
        {
            case sysExWish_get:
            case sysExWish_backup:
            return ML_REQ_STANDARD;

            case sysExWish_set:
            size = sysExMessage[decodedMessage.block].section[decodedMessage.section].numberOfParameters;

            if (size > PARAMETERS_PER_MESSAGE)
            {
                if ((decodedMessage.part+1) == sysExMessage[decodedMessage.block].section[decodedMessage.section].parts)
                    size = size - ((sysExMessage[decodedMessage.block].section[decodedMessage.section].parts-1)*PARAMETERS_PER_MESSAGE);
                else
                    size = PARAMETERS_PER_MESSAGE;
            }

            size *= sizeof(sysExParameter_t);
            size += ML_REQ_STANDARD;
            return size;

            default:
            return 0;
        }
        break;

        default:
        return 0;
    }

    return size;
}

///
/// \brief Checks whether the wish value is valid.
/// \returns    True if valid, false otherwise.
///
bool SysEx::checkWish()
{
    return (decodedMessage.wish < SYSEX_WISH_MAX);
}

///
/// \brief Checks whether the amount value is valid.
/// \returns    True if valid, false otherwise.
///
bool SysEx::checkAmount()
{
    return (decodedMessage.amount < SYSEX_AMOUNT_MAX);
}

///
/// \brief Checks whether the block value is valid.
/// \returns    True if valid, false otherwise.
///
bool SysEx::checkBlock()
{
    return decodedMessage.block < sysExBlockCounter;
}

///
/// \brief Checks whether the section value is valid.
/// \returns    True if valid, false otherwise.
///
bool SysEx::checkSection()
{
    return (decodedMessage.section < sysExMessage[decodedMessage.block].numberOfSections);
}

///
/// \brief Checks whether the message part is valid.
/// \returns    True if valid, false otherwise.
///
bool SysEx::checkPart()
{
    switch(decodedMessage.wish)
    {
        case sysExWish_get:
        if (decodedMessage.part == 127)
            return true;

        if (decodedMessage.part >= sysExMessage[decodedMessage.block].section[decodedMessage.section].parts)
            return false;
        else
            return true;
        break;

        case sysExWish_set:
        case sysExWish_backup:
        if (decodedMessage.wish == sysExWish_backup)
        {
            if (decodedMessage.part == 127)
                return true;
        }

        if (decodedMessage.amount == sysExAmount_all)
        {
            if (decodedMessage.part < sysExMessage[decodedMessage.block].section[decodedMessage.section].parts)
                return true;
            else
                return false;
        }
        else
        {
            if (decodedMessage.part >= sysExMessage[decodedMessage.block].section[decodedMessage.section].parts)
                return false;
            else
                return true;
        }
        break;

        default:
        return false;
    }
}

///
/// \brief Checks whether the parameter index in message is valid.
/// \returns    True if valid, false otherwise.
///
bool SysEx::checkParameterIndex()
{
    //block and section passed validation, check parameter index
    return (decodedMessage.index < sysExMessage[decodedMessage.block].section[decodedMessage.section].numberOfParameters);
}

///
/// \brief Checks whether the new value in message is valid.
/// \returns    True if valid, false otherwise.
///
bool SysEx::checkNewValue()
{
    sysExParameter_t minValue = sysExMessage[decodedMessage.block].section[decodedMessage.section].minValue;
    sysExParameter_t maxValue = sysExMessage[decodedMessage.block].section[decodedMessage.section].maxValue;

    if (minValue != maxValue)
        return ((decodedMessage.newValue >= minValue) && (decodedMessage.newValue <= maxValue));
    else
        return true; //don't check new value if min and max are the same
}

///
/// \brief Starts custom SysEx response.
///
void SysEx::startResponse()
{
    responseSize = 0;

    sysExArray[responseSize] = 0xF0;
    responseSize++;
    sysExArray[responseSize] = SYS_EX_M_ID_0;
    responseSize++;
    sysExArray[responseSize] = SYS_EX_M_ID_1;
    responseSize++;
    sysExArray[responseSize] = SYS_EX_M_ID_2;
    responseSize++;
    sysExArray[responseSize] = ACK;
    responseSize++;
    sysExArray[responseSize] = 0; //message part
    responseSize++;
}

///
/// \brief Adds value to custom SysEx response.
/// @param [in] value   New value.
///
void SysEx::addToResponse(sysExParameter_t value)
{
    #if PARAM_SIZE == 2
    encDec_14bit encoded;
    encoded.value = value;
    encoded.encodeTo14bit();
    sysExArray[responseSize] = encoded.high;
    responseSize++;
    sysExArray[responseSize] = encoded.low;
    responseSize++;
    #elif PARAM_SIZE == 1
    sysExArray[responseSize] = (uint8_t)value;
    responseSize++;
    #endif
}

///
/// \brief Sends built SysEx response.
///
void SysEx::sendResponse()
{
    sysExArray[responseSize] = 0xF7;
    responseSize++;

    midi.sendSysEx(responseSize, sysExArray, true);
}

void SysEx::setStatus(sysExStatus_t status)
{
    sysExArray[statusByte] = (uint8_t)status;
}

///
/// \brief Handler used to set callback function for get requests.
///
void SysEx::setHandleGet(sysExParameter_t(*fptr)(uint8_t block, uint8_t section, uint16_t index))
{
    sendGetCallback = fptr;
}

///
/// \brief Handler used to set callback function for set requests.
///
void SysEx::setHandleSet(bool(*fptr)(uint8_t block, uint8_t section, uint16_t index, sysExParameter_t newValue))
{
    sendSetCallback = fptr;
}

///
/// \brief Handler used to set callback function for custom requests.
///
void SysEx::setHandleCustomRequest(bool(*fptr)(uint8_t value)) 
{
    sendCustomRequestCallback = fptr;
}

SysEx sysEx;
