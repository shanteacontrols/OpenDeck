#include <avr/eeprom.h>
#include "SysEx.h"
#include "../hardware/board/Board.h"

SysEx::SysEx()  {

    sendRebootCallback          = NULL;
    sendFactoryResetCallback    = NULL;
    sendGetCallback             = NULL;
    sendSetCallback             = NULL;
    sendResetCallback           = NULL;

    sysExEnabled = false;

    for (int i=0; i<MAX_NUMBER_OF_MESSAGES; i++)    {

        messageInfo[i].messageTypeID = INVALID_VALUE;
        messageInfo[i].numberOfSubtypes = INVALID_VALUE;

        for (int j=0; j<MAX_NUMBER_OF_SUBTYPES; j++)    {

            for (int k=0; k<SUBTYPE_FIELDS; k++)
                messageInfo[i].subTypeInfo[j][k] = INVALID_VALUE;

        }

    }

}

void SysEx::enableConf()    {

    sysExEnabled = true;

}

void SysEx::disableConf()   {

    sysExEnabled = false;

}

void SysEx::addMessageType(uint8_t messageID, uint8_t subTypes) {

    messageInfo[messageID].messageTypeID = messageID;
    messageInfo[messageID].numberOfSubtypes = subTypes;

}

void SysEx::addMessageSubType(uint8_t messageID, uint8_t subTypeId, uint8_t numberOfParameters, uint8_t minValue, uint8_t maxValue) {

    messageInfo[messageID].subTypeInfo[subTypeId][0] = numberOfParameters;
    messageInfo[messageID].subTypeInfo[subTypeId][1] = minValue;
    messageInfo[messageID].subTypeInfo[subTypeId][2] = maxValue;

}

bool SysEx::checkSpecial(uint8_t *array, uint8_t size) {

    if (size == ML_SPECIAL)   {

        if (array[size-2] == REBOOT_STRING)  {   //reset message

            if (sysExEnabled)
                sendRebootCallback();
            else sendError(ERROR_HANDSHAKE);
            return true;

        }   else if (array[size-2] == HELLO_STRING)   {

            //hello message, necessary for allowing configuration
            sendHelloResponse();
            return true;

        }   else if (array[size-2] == FACTORY_RESET_STRING) {

            sendFactoryResetCallback();

        }

    }   return false;

}

void SysEx::handleSysEx(uint8_t *sysExArray, uint8_t size)    {

    //we have new data
    dataAvailable = true;

    //ignore messages shorter than absolute minimum
    if (size < ML_SPECIAL) return;
    //don't respond to sysex message if device ID is wrong
    sysExManufacturerID id;
    id.byte1 = sysExArray[MS_M_ID_0];
    id.byte2 = sysExArray[MS_M_ID_1];
    id.byte3 = sysExArray[MS_M_ID_2];

    if (!checkID(id)) return;

    if (checkSpecial(sysExArray, size)) return;
    //message appears to be fine for now
    //check if hello message has been received by now
    if (!sysExEnabled) {

        //message is fine, but handshake hasn't been received
        sendError(ERROR_HANDSHAKE);
        return;

    }

    if (!checkMessageValidity(sysExArray, size)) return; //message not valid
        sendResponse(sysExArray, size);

    //data is processed
    dataAvailable = false;

}

bool SysEx::checkMessageValidity(uint8_t sysExArray[], uint8_t arrSize)  {

    //check wish validity
    if (!checkWish(sysExArray[MS_WISH]))    {

        sendError(ERROR_WISH);
        return false;

    }

    //check if wanted amount is correct
    if (!checkAmount(sysExArray[MS_AMOUNT]))    {

        sendError(ERROR_AMOUNT);
        return false;

    }

    //check if message type is correct
    if (!checkBlock(sysExArray[MS_BLOCK]))    {

        sendError(ERROR_BLOCK);
        return false;

    } else {

        //determine minimum message length based on asked parameters
        if (arrSize < generateMinMessageLenght(sysExArray[MS_WISH], sysExArray[MS_AMOUNT],sysExArray[MS_BLOCK], sysExArray[MS_SECTION]))    {

            sendError(ERROR_MESSAGE_LENGTH);
            return false;

        }

    }

    //check if subtype is correct
    if (!checkSection(sysExArray[MS_BLOCK], sysExArray[MS_SECTION])) {

        sendError(ERROR_SECTION);
        return false;

    }

    //check if wanted parameter is valid only if single parameter is specified
    if (sysExArray[MS_AMOUNT] == AMOUNT_SINGLE)   {

        if (!checkParameterID(sysExArray[MS_BLOCK], sysExArray[MS_SECTION], sysExArray[MS_PARAMETER_ID]))  {

            sendError(ERROR_PARAMETER);
            return false;

        }

        //if message wish is set, check new parameter
        if (sysExArray[MS_WISH] == WISH_SET) {

            if (!checkNewParameter(sysExArray[MS_BLOCK], sysExArray[MS_SECTION], sysExArray[MS_PARAMETER_ID], sysExArray[MS_NEW_PARAMETER_ID_SINGLE]))  {

                sendError(ERROR_NEW_PARAMETER);
                return false;

            }

        }

    } else {

        //all parameters

        //check each new parameter for set command
        if (sysExArray[MS_WISH] == WISH_SET) {

            uint8_t arrayIndex = MS_NEW_PARAMETER_ID_ALL;

            for (int i=0; i<(arrSize - arrayIndex)-1; i++)

            if (!checkNewParameter(sysExArray[MS_BLOCK], sysExArray[MS_SECTION], i, sysExArray[arrayIndex+i]))   {

                sendError(ERROR_NEW_PARAMETER);
                return false;

            }

        }

    }

    return true;

}

bool SysEx::checkID(sysExManufacturerID id)   {

    return  (

    (id.byte1  == SYS_EX_M_ID_0)   &&
    (id.byte2 == SYS_EX_M_ID_1)   &&
    (id.byte3  == SYS_EX_M_ID_2)

    );

}

bool SysEx::checkWish(uint8_t wish)   {

    return ((wish >= WISH_START) && (wish < WISH_END));

}

bool SysEx::checkAmount(uint8_t amount)    {

    return ((amount >= AMOUNT_START) && (amount < AMOUNT_END));

}


//component specific

bool SysEx::checkBlock(uint8_t messageType) {

    //check if message type is valid
    for (int i=0; i<MAX_NUMBER_OF_MESSAGES; i++)
        if (messageInfo[messageType].messageTypeID == messageType)
            return true;

    return false;

}

bool SysEx::checkSection(uint8_t messageType, uint8_t messageSubType)    {

    return (messageSubType < messageInfo[messageType].numberOfSubtypes);

}

bool SysEx::checkParameterID(uint8_t messageType, uint8_t messageSubType, uint8_t parameter)   {

    //message type and subtype passed validation, check parameter ID
    return (parameter < messageInfo[messageType].subTypeInfo[messageSubType][PARAMETERS_BYTE]);

}

bool SysEx::checkNewParameter(uint8_t messageType, uint8_t messageSubType, uint8_t parameter, uint8_t newParameter) {

    uint8_t minValue = messageInfo[messageType].subTypeInfo[messageSubType][NEW_VALUE_MIN_BYTE];
    uint8_t maxValue = messageInfo[messageType].subTypeInfo[messageSubType][NEW_VALUE_MAX_BYTE];

    if ((minValue != IGNORE_NEW_VALUE) && (maxValue != IGNORE_NEW_VALUE))
        return ((newParameter >= minValue) && (newParameter <= maxValue));
    else if ((minValue != IGNORE_NEW_VALUE) && (maxValue == IGNORE_NEW_VALUE))
        return (newParameter >= minValue); //check only min value
    else if ((minValue == IGNORE_NEW_VALUE) && (maxValue != IGNORE_NEW_VALUE))
        return (newParameter <= maxValue);   //check only max value
    else if ((minValue == IGNORE_NEW_VALUE) && (maxValue == IGNORE_NEW_VALUE))
        return true; //don't check new parameter

    return false;

}


uint8_t SysEx::generateMinMessageLenght(uint8_t wish, uint8_t amount, uint8_t messageType, uint8_t messageSubType)    {

    //single parameter
    if (amount == AMOUNT_SINGLE)  {

        if ((wish == WISH_GET) ||
        (wish == WISH_RESTORE)) return ML_REQ_STANDARD + 1;  //get   //add 1 to length for parameter
        else                    return ML_REQ_STANDARD + 2;  //set   //add 2 to length for parameter and new value

        }   else if (amount == AMOUNT_ALL)   {

        if ((wish == WISH_GET) || (wish == WISH_RESTORE))             //get/restore
            return ML_REQ_STANDARD;

        else    {                                                                   //set

            return ML_REQ_STANDARD + messageInfo[messageType].subTypeInfo[messageSubType][PARAMETERS_BYTE];

        }

    }   else return 0;

}

void SysEx::sendError(sysExError errorID)  {

    //public function, interface objects can use this function directly
    //to override internal error checking

    uint8_t sysExResponse[5];

    sysExResponse[0] = SYS_EX_M_ID_0;
    sysExResponse[1] = SYS_EX_M_ID_1;
    sysExResponse[2] = SYS_EX_M_ID_2;
    sysExResponse[3] = RESPONSE_NACK;
    sysExResponse[4] = errorID;

    midi.sendSysEx(sysExResponse, 5);
    dataAvailable = false;

}

void SysEx::sendHelloResponse()   {

    uint8_t sysExAckResponse[7];

    sysExAckResponse[0] = SYS_EX_M_ID_0;
    sysExAckResponse[1] = SYS_EX_M_ID_1;
    sysExAckResponse[2] = SYS_EX_M_ID_2;
    sysExAckResponse[3] = RESPONSE_ACK;
    sysExAckResponse[4] = getSWversion(swVersion_major);
    sysExAckResponse[5] = getSWversion(swVersion_minor);
    sysExAckResponse[6] = getSWversion(swVersion_revision);

    sysExEnabled = true;

    midi.sendSysEx(sysExAckResponse, 7);

}

void SysEx::sendComponentID(uint8_t blockID, uint8_t componentID)   {

    uint8_t sysExResponse[6];

    sysExResponse[0] = SYS_EX_M_ID_0;
    sysExResponse[1] = SYS_EX_M_ID_1;
    sysExResponse[2] = SYS_EX_M_ID_2;
    sysExResponse[3] = COMPONENT_INFO_STRING;
    sysExResponse[4] = blockID;
    sysExResponse[5] = componentID;

    midi.sendSysEx(sysExResponse, 6);

}

void SysEx::sendResponse(uint8_t sysExArray[], uint8_t arraySize)  {

    uint8_t componentNr     = 1,
            _parameter      = 0;

    //create basic response
    uint8_t sysExResponse[64+ML_RES_BASIC];
    uint8_t arrayLength = 0;
    uint8_t arrayIndex = 0;
    bool eepromError = false;

    sysExResponse[0] = SYS_EX_M_ID_0;
    sysExResponse[1] = SYS_EX_M_ID_1;
    sysExResponse[2] = SYS_EX_M_ID_2;
    sysExResponse[3] = RESPONSE_ACK;

    if (sysExArray[MS_AMOUNT] == AMOUNT_ALL) {

        uint8_t messageType = sysExArray[MS_BLOCK];
        uint8_t messageSubtype = sysExArray[MS_SECTION];
        componentNr = messageInfo[messageType].subTypeInfo[messageSubtype][PARAMETERS_BYTE];

    }

    //create response based on wanted message type
    switch(sysExArray[MS_WISH]) {

        case WISH_GET:
        if (sysExArray[MS_AMOUNT] == AMOUNT_ALL)    _parameter = 0;
        else                                        _parameter = sysExArray[MS_PARAMETER_ID];

        for (int i=0; i<componentNr; i++) {

            sysExResponse[i+ML_SET_RESTORE] = sendGetCallback(sysExArray[MS_BLOCK], sysExArray[MS_SECTION], _parameter);
            _parameter++;

        }   arrayLength = ML_SET_RESTORE+componentNr;
        break;

        case WISH_SET:
        if (sysExArray[MS_AMOUNT] == AMOUNT_ALL) {

            _parameter = 0;
            arrayIndex = MS_NEW_PARAMETER_ID_ALL;

            }   else    {

            _parameter = sysExArray[MS_PARAMETER_ID];
            arrayIndex = MS_NEW_PARAMETER_ID_SINGLE;

        }

        for (int i=0; i<componentNr; i++)   {

            if (!sendSetCallback(sysExArray[MS_BLOCK], sysExArray[MS_SECTION], _parameter, sysExArray[arrayIndex+i]))  {

                eepromError = true;
                break;

            }

            _parameter++;

        }   arrayLength = ML_SET_RESTORE;
        break;

        case WISH_RESTORE:
        if (sysExArray[MS_AMOUNT] == AMOUNT_ALL)
        _parameter = 0;
        else _parameter = sysExArray[MS_PARAMETER_ID];

        for (int i=0; i<componentNr; i++)   {

            if (!sendResetCallback(sysExArray[MS_BLOCK], sysExArray[MS_SECTION], _parameter))  {

                eepromError = true;
                break;

            }   _parameter++;

        }   arrayLength = ML_SET_RESTORE;
        break;

    }

    if (dataAvailable)  {

        if (!eepromError)   midi.sendSysEx(sysExResponse, arrayLength);
        else                sendError(ERROR_EEPROM);

    }

}

//callbacks

void SysEx::setHandleReboot(void (*fptr)(void)) {

    sendRebootCallback = fptr;

}

void SysEx::setHandleFactoryReset(void (*fptr)(void))   {

    sendFactoryResetCallback = fptr;

}

void SysEx::setHandleGet(uint8_t(*fptr)(uint8_t messageID, uint8_t messageSubtype, uint8_t parameter))    {

    sendGetCallback = fptr;

}

void SysEx::setHandleSet(bool(*fptr)(uint8_t messageType, uint8_t messageSubType, uint8_t parameterID, uint8_t newParameterID))    {

    sendSetCallback = fptr;

}

void SysEx::setHandleReset(bool(*fptr)(uint8_t messageID, uint8_t messageSubtype, uint8_t parameter))    {

    sendResetCallback = fptr;

}

bool SysEx::configurationEnabled()  {

    return sysExEnabled;

}

SysEx sysEx;
