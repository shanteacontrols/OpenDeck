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

#include "SysEx.h"

#ifdef SYSEX_H_

const sysExManufacturerID defaultID = {

    SYS_EX_M_ID_0,
    SYS_EX_M_ID_1,
    SYS_EX_M_ID_2

};

SysEx::SysEx()  {

    sendGetCallback             = NULL;
    sendSetCallback             = NULL;
    sendCustomRequestCallback   = NULL;

    sysExEnabled = false;
    forcedSend = false;

    for (int i=0; i<MAX_NUMBER_OF_BLOCKS; i++)    {

        sysExMessage[i].numberOfSections = INVALID_VALUE;
        sysExMessage[i].sectionCounter = 0;

        for (int j=0; j<MAX_NUMBER_OF_SECTIONS; j++)    {

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

void SysEx::enableConf()    {

    sysExEnabled = true;

}

void SysEx::disableConf()   {

    sysExEnabled = false;

}

bool SysEx::configurationEnabled()  {

    return sysExEnabled;

}

bool SysEx::addCustomRequest(uint8_t value)  {

    if (customRequestCounter >= MAX_CUSTOM_REQUESTS) return false;

    //don't add custom string if it's already defined as one of default strings
    if (value < SPECIAL_PARAMETERS) return false;

    customRequests[customRequestCounter] = value;
    customRequestCounter++;
    return true;

}

bool SysEx::addBlock(uint8_t sections) {

    if (sysExBlockCounter >= MAX_NUMBER_OF_BLOCKS) return false;

    sysExMessage[sysExBlockCounter].numberOfSections = sections;
    sysExBlockCounter++;
    return true;

}

bool SysEx::addSection(uint8_t block, sysExParameter_t numberOfParameters, sysExParameter_t minValue, sysExParameter_t maxValue) {

    if (sysExMessage[block].sectionCounter >= MAX_NUMBER_OF_SECTIONS) return false;

    sysExMessage[block].section[sysExMessage[block].sectionCounter].numberOfParameters = numberOfParameters;
    sysExMessage[block].section[sysExMessage[block].sectionCounter].minValue = minValue;
    sysExMessage[block].section[sysExMessage[block].sectionCounter].maxValue = maxValue;

    //based on number of parameters, calculate how many parts message has in case of set/all request and get/all response
    sysExMessage[block].section[sysExMessage[block].sectionCounter].parts = sysExMessage[block].section[sysExMessage[block].sectionCounter].numberOfParameters / PARAMETERS_PER_MESSAGE;
    if (sysExMessage[block].section[sysExMessage[block].sectionCounter].numberOfParameters % PARAMETERS_PER_MESSAGE)
        sysExMessage[block].section[sysExMessage[block].sectionCounter].parts++;

    sysExMessage[block].sectionCounter++;
    return true;

}

void SysEx::handleSysEx(uint8_t *array, uint8_t size)    {

    //save pointer to received array so we can manipulate it directly
    sysExArray = array;
    sysExArraySize = size;
    responseSize = RESPONSE_SIZE;

    if (size < MIN_MESSAGE_LENGTH) return; //ignore small messages

    //don't respond to sysex message if device ID is wrong
    decodedMessage.id.byte1 = sysExArray[idByte_1];
    decodedMessage.id.byte2 = sysExArray[idByte_2];
    decodedMessage.id.byte3 = sysExArray[idByte_3];
    decodedMessage.status = (sysExStatus_t)sysExArray[(uint8_t)statusByte];

    if (decodedMessage.status != REQUEST)   {

        //don't let status be anything but request
        setStatus(ERROR_STATUS);

    }   else {

        if (checkID()) {

            if (!checkSpecialRequests()) {

                //message appears to be fine for now
                if (sysExEnabled) {

                    if (sysExArraySize < REQUEST_SIZE) {

                        setStatus(ERROR_MESSAGE_LENGTH);

                    }   else {

                        //don't try to request these parameters if the size is too small
                        decodedMessage.part = sysExArray[(uint8_t)partByte];
                        decodedMessage.wish = (sysExWish)sysExArray[(uint8_t)wishByte];
                        decodedMessage.amount = (sysExAmount)sysExArray[(uint8_t)amountByte];
                        decodedMessage.block = sysExArray[(uint8_t)blockByte];
                        decodedMessage.section = sysExArray[(uint8_t)sectionByte];

                        if (checkRequest()) {

                            if (size < generateMinMessageLenght())    {

                                setStatus(ERROR_MESSAGE_LENGTH);

                            } else {

                                if (checkParameters())  {

                                    //message is ok

                                }

                            }

                        }

                    }

                }   else {

                    //message is fine, but handshake hasn't been received
                    setStatus(ERROR_HANDSHAKE);

                }

            }

        }   else {

            //don't send response to wrong ID

        }

    }

    if (!forcedSend)    {

        sysExArray[responseSize] = 0xF7;
        responseSize++;

        midi.sendSysEx(responseSize, sysExArray, true);

    }

    forcedSend = false;

}

bool SysEx::checkID()   {

    if (sysExArraySize < (idByte_3+1)) return false;

    return  (

    (decodedMessage.id.byte1  == defaultID.byte1)   &&
    (decodedMessage.id.byte2 == defaultID.byte2)   &&
    (decodedMessage.id.byte3== defaultID.byte3)

    );

}

bool SysEx::checkSpecialRequests() {

    if (sysExArraySize == MIN_MESSAGE_LENGTH)   {

        switch(sysExArray[wishByte])  {

            case HANDSHAKE_REQUEST:
            //hello message, necessary for allowing configuration
            sysExEnabled = true;
            setStatus(ACK);
            return true;

            case BYTES_PER_VALUE_REQUEST:
            if (sysExEnabled)   {

                setStatus(ACK);
                addToResponse(PARAM_SIZE);

            }   else {

                setStatus(ERROR_HANDSHAKE);

            }
            return true;

            case PARAMS_PER_MESSAGE_REQUEST:
            if (sysExEnabled)   {

                setStatus(ACK);
                addToResponse(PARAMETERS_PER_MESSAGE);

            }   else {

                setStatus(ERROR_HANDSHAKE);

            }
            return true;

            default:
            //check for custom string
            for (int i=0; i<MAX_CUSTOM_REQUESTS; i++) {

                if (customRequests[i] != INVALID_VALUE)  {

                    if (customRequests[i] == sysExArray[wishByte])    {

                        if (sysExEnabled)   {

                            setStatus(ACK);
                            sendCustomRequestCallback(customRequests[i]);

                        }   else {

                            setStatus(ERROR_HANDSHAKE);

                        }   return true;

                    }

                }

            }
            setStatus(ERROR_WISH);
            return true;
            break;

        }

    }

    return false;

}

bool SysEx::checkRequest()  {

    if (!checkWish())    {

        setStatus(ERROR_WISH);
        return false;

    }

    if (!checkBlock())    {

        setStatus(ERROR_BLOCK);
        return false;

    }

    if (!checkSection()) {

        setStatus(ERROR_SECTION);
        return false;

    }

    if (!checkAmount())    {

        setStatus(ERROR_AMOUNT);
        return false;

    }

    if (!checkPart())   {

        setStatus(ERROR_PART);
        return false;

    }

    return true;

}

bool SysEx::checkParameters()   {

    //sysex request is fine
    //start building response
    setStatus(ACK);

    uint8_t loops = 1, responseSize_ = responseSize;
    uint16_t startIndex = 0, endIndex = 1;

    if (decodedMessage.amount == sysExAmount_single)   {

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

        if (decodedMessage.wish == sysExWish_set)   {

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

    if ((decodedMessage.wish == sysExWish_backup) || (decodedMessage.wish == sysExWish_get))    {

        if (decodedMessage.part == 127) {

            loops = sysExMessage[decodedMessage.block].section[decodedMessage.section].parts;
            forcedSend = true;

        }

        if (decodedMessage.wish == sysExWish_backup)    {

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

    for (int j=0; j<loops; j++) {

        responseSize = responseSize_;

        if (forcedSend)  {

            decodedMessage.part = j;
            sysExArray[partByte] = j;

        }

        if (decodedMessage.amount == sysExAmount_all)   {

            startIndex = PARAMETERS_PER_MESSAGE*decodedMessage.part;
            endIndex = startIndex + PARAMETERS_PER_MESSAGE;

            if ((sysExParameter_t)endIndex > sysExMessage[decodedMessage.block].section[decodedMessage.section].numberOfParameters)
                endIndex = sysExMessage[decodedMessage.block].section[decodedMessage.section].numberOfParameters;

        }

        for (uint16_t i=startIndex; i<endIndex; i++)    {

            switch(decodedMessage.wish) {

                case sysExWish_get:
                if (decodedMessage.amount == sysExAmount_single)    {

                    if (!checkParameterIndex())  {

                        setStatus(ERROR_INDEX);
                        return false;

                    }   else {

                        addToResponse(sendGetCallback(decodedMessage.block, decodedMessage.section, decodedMessage.index));

                    }

                }   else {

                    //get all params - no index is specified
                    addToResponse(sendGetCallback(decodedMessage.block, decodedMessage.section, i));

                }
                break;

                case sysExWish_set:
                if (decodedMessage.amount == sysExAmount_single)    {

                    if (!checkParameterIndex())  {

                        setStatus(ERROR_INDEX);
                        return false;

                    }

                    if (!checkNewValue())   {

                        setStatus(ERROR_NEW_VALUE);
                        return false;

                    }

                    if (sendSetCallback(decodedMessage.block, decodedMessage.section, decodedMessage.index, decodedMessage.newValue))   {

                        return true;

                    }

                }   else {

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

                    if (!checkNewValue())   {

                        setStatus(ERROR_NEW_VALUE);
                        return false;

                    }

                    if (!sendSetCallback(decodedMessage.block, decodedMessage.section, i, decodedMessage.newValue))  {

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

        if (forcedSend) {

            sysExArray[responseSize] = 0xF7;
            responseSize++;

            midi.sendSysEx(responseSize, sysExArray, true);

        }

    }

    return true;

}

uint8_t SysEx::generateMinMessageLenght()    {

    uint16_t size = 0;

    switch(decodedMessage.amount)   {

        case sysExAmount_single:
        switch(decodedMessage.wish) {

            case sysExWish_get:
            case sysExWish_backup:
            size = ML_REQ_STANDARD + sizeof(sysExParameter_t); //add parameter length
            break;

            case sysExWish_set:
            size = ML_REQ_STANDARD + 2*sizeof(sysExParameter_t); //add parameter length and new value length
            break;

            default:
            break;

        }
        break;

        case sysExAmount_all:
        switch(decodedMessage.wish) {

            case sysExWish_get:
            case sysExWish_backup:
            size = ML_REQ_STANDARD;
            break;

            case sysExWish_set:
            size = sysExMessage[decodedMessage.block].section[decodedMessage.section].numberOfParameters;
            if (size > PARAMETERS_PER_MESSAGE) {

                if ((decodedMessage.part+1) == sysExMessage[decodedMessage.block].section[decodedMessage.section].parts)    {

                    size = size - ((sysExMessage[decodedMessage.block].section[decodedMessage.section].parts-1)*PARAMETERS_PER_MESSAGE);

                } else {

                    size = PARAMETERS_PER_MESSAGE;

                }

            }
            size *= sizeof(sysExParameter_t);
            size += ML_REQ_STANDARD;
            break;

            default:
            break;

        }
        break;

        default:
        break;

    }

    return size;

}

bool SysEx::checkWish()   {

    return (decodedMessage.wish < SYSEX_WISH_MAX);

}

bool SysEx::checkAmount()    {

    return (decodedMessage.amount < SYSEX_AMOUNT_MAX);

}

bool SysEx::checkBlock() {

    return decodedMessage.block < sysExBlockCounter;

}

bool SysEx::checkSection()    {

    return (decodedMessage.section < sysExMessage[decodedMessage.block].numberOfSections);

}

bool SysEx::checkPart() {

    switch(decodedMessage.wish) {

        case sysExWish_get:
        if (decodedMessage.part == 127) return true;
        if (decodedMessage.part >= sysExMessage[decodedMessage.block].section[decodedMessage.section].parts)   {

            return false;

        }   else {

            return true;

        }
        break;

        case sysExWish_set:
        case sysExWish_backup:
        if (decodedMessage.wish == sysExWish_backup)    {

            if (decodedMessage.part == 127)
                return true;

        }
        if (decodedMessage.amount == sysExAmount_all)   {

            if (decodedMessage.part < sysExMessage[decodedMessage.block].section[decodedMessage.section].parts) {

                return true;

            }   else {

                return false;

            }

        }   else {

            if (decodedMessage.part >= sysExMessage[decodedMessage.block].section[decodedMessage.section].parts)   {

                return false;

            }   else {

                return true;

            }

        }
        break;

        default:
        return false;

    }

}

bool SysEx::checkParameterIndex()   {

    //block and section passed validation, check parameter index
    return (decodedMessage.index < sysExMessage[decodedMessage.block].section[decodedMessage.section].numberOfParameters);

}

bool SysEx::checkNewValue() {

    sysExParameter_t minValue = sysExMessage[decodedMessage.block].section[decodedMessage.section].minValue;
    sysExParameter_t maxValue = sysExMessage[decodedMessage.block].section[decodedMessage.section].maxValue;

    if (minValue != maxValue)
        return ((decodedMessage.newValue >= minValue) && (decodedMessage.newValue <= maxValue));
    else return true; //don't check new value if min and max are the same

}

void SysEx::addToResponse(sysExParameter_t value)    {

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

void SysEx::sendCustomMessage(uint8_t id, sysExParameter_t value)   {

    #if PARAM_SIZE == 2
    uint8_t size = 9;
    #elif PARAM_SIZE == 1
    uint8_t size = 8;
    #endif

    uint8_t customMessage[size];

    customMessage[startByte] = 0xF0;
    customMessage[idByte_1] = defaultID.byte1;
    customMessage[idByte_2] = defaultID.byte2;
    customMessage[idByte_3] = defaultID.byte3;
    customMessage[statusByte] = ACK;
    customMessage[statusByte+1] = id;
    #if PARAM_SIZE == 2
    encDec_14bit encoded;
    encoded.value = value;
    encoded.encodeTo14bit();
    customMessage[statusByte+2] = encoded.high;
    customMessage[statusByte+3] = encoded.low;
    customMessage[statusByte+4] = 0xF7;
    #elif PARAM_SIZE == 1
    customMessage[statusByte+2] = value;
    customMessage[statusByte+3] = 0xF7;
    #endif

    midi.sendSysEx(size, customMessage, true);

}

void SysEx::setStatus(sysExStatus_t status)    {

    sysExArray[statusByte] = (uint8_t)status;

}

//callbacks

void SysEx::setHandleGet(sysExParameter_t(*fptr)(uint8_t block, uint8_t section, uint16_t index))    {

    sendGetCallback = fptr;

}

void SysEx::setHandleSet(bool(*fptr)(uint8_t block, uint8_t section, uint16_t index, sysExParameter_t newValue))    {

    sendSetCallback = fptr;

}

void SysEx::setHandleCustomRequest(bool(*fptr)(uint8_t value))  {

    sendCustomRequestCallback = fptr;

}

SysEx sysEx;
#endif