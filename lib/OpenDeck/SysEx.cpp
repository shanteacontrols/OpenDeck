/*

OpenDECK library v1.3
File: SysEx.cpp
Last revision date: 2014-12-25
Author: Igor Petrovic

*/

#include "OpenDeck.h"
#include <avr/eeprom.h>
#include "Ownduino.h"

void OpenDeck::setHandleSysExSend(void (*fptr)(uint8_t *sysExArray, uint8_t size))  {

    sendSysExDataCallback = fptr;

}

void OpenDeck::processSysEx(uint8_t sysExArray[], uint8_t arrSize)  {

    if (sysExCheckMessageValidity(sysExArray, arrSize))
        sysExGenerateResponse(sysExArray, arrSize);

}

bool OpenDeck::sysExCheckMessageValidity(uint8_t sysExArray[], uint8_t arrSize)  {

    //don't respond to sysex message if device ID is wrong
    if (sysExCheckID(sysExArray[SYS_EX_MS_M_ID_0], sysExArray[SYS_EX_MS_M_ID_1], sysExArray[SYS_EX_MS_M_ID_2]))  {

        //only check rest of the message if it's not just a ID check and controller has received handshake
        if ((arrSize >= SYS_EX_ML_REQ_STANDARD) && (sysExEnabled))    {

            //check wish validity
            if (sysExCheckWish(sysExArray[SYS_EX_MS_WISH]))    {

                //check if wanted amount is correct
                if (sysExCheckAmount(sysExArray[SYS_EX_MS_AMOUNT])) {

                    //check if message type is correct
                    if (sysExCheckMessageType(sysExArray[SYS_EX_MS_MT])) {

                        //check for special wish/amount/message type combinations
                        if (!(sysExCheckSpecial(sysExArray[SYS_EX_MS_MT], sysExArray[SYS_EX_MS_WISH], sysExArray[SYS_EX_MS_AMOUNT])))
                            return false;

                        //determine minimum message length based on asked parameters
                        if (arrSize < sysExGenerateMinMessageLenght(sysExArray[SYS_EX_MS_WISH],
                                                                    sysExArray[SYS_EX_MS_AMOUNT],
                                                                    sysExArray[SYS_EX_MS_MT],
                                                                    sysExArray[SYS_EX_MS_MST]))    {

                            sysExGenerateError(SYS_EX_ERROR_MESSAGE_LENGTH);
                            return false;

                        }

                        //check if subtype is correct
                        if (sysExCheckMessageSubType(sysExArray[SYS_EX_MS_MT], sysExArray[SYS_EX_MS_MST]))   {

                            //check if wanted parameter is valid only if single parameter is specified
                            if (sysExArray[SYS_EX_MS_AMOUNT] == SYS_EX_AMOUNT_SINGLE) {

                                if (sysExCheckParameterID(sysExArray[SYS_EX_MS_MT], sysExArray[SYS_EX_MS_MST], sysExArray[SYS_EX_MS_PARAMETER_ID]))  {

                                    //if message wish is set, check new parameter
                                    if (sysExArray[SYS_EX_MS_WISH] == SYS_EX_WISH_SET) {

                                        if (!sysExCheckNewParameterID(  sysExArray[SYS_EX_MS_MT],
                                                                        sysExArray[SYS_EX_MS_MST],
                                                                        sysExArray[SYS_EX_MS_PARAMETER_ID],
                                                                        sysExArray[SYS_EX_MS_NEW_PARAMETER_ID_SINGLE]))   {

                                            sysExGenerateError(SYS_EX_ERROR_NEW_PARAMETER);
                                            return false;

                                        }

                                    }

                                }   else {

                                    sysExGenerateError(SYS_EX_ERROR_PARAMETER);
                                    return false;

                                }

                            }   else    {   //all parameters

                                    //check each new parameter for set command
                                    if (sysExArray[SYS_EX_MS_WISH] == SYS_EX_WISH_SET) {

                                        uint8_t arrayIndex = SYS_EX_MS_NEW_PARAMETER_ID_ALL;

                                        for (int i=0; i<(arrSize - arrayIndex)-1; i++)

                                        if (!sysExCheckNewParameterID(  sysExArray[SYS_EX_MS_MT],
                                                                        sysExArray[SYS_EX_MS_MST],
                                                                        i,
                                                                        sysExArray[arrayIndex+i]))   {

                                            sysExGenerateError(SYS_EX_ERROR_NEW_PARAMETER);
                                            return false;

                                         }

                                    }

                                }

                        }  else {

                                sysExGenerateError(SYS_EX_ERROR_MST);
                                return false;

                            }

                        }   else {

                                sysExGenerateError(SYS_EX_ERROR_MT);
                                return false;

                            }

                    }   else {

                            sysExGenerateError(SYS_EX_ERROR_AMOUNT);
                            return false;

                        }

                }   else {

                        sysExGenerateError(SYS_EX_ERROR_WISH);
                        return false;

                    }

            }   else    {

                    if (arrSize != SYS_EX_ML_REQ_HANDSHAKE) {

                        if (sysExEnabled)   sysExGenerateError(SYS_EX_ERROR_MESSAGE_LENGTH);
                        else                sysExGenerateError(SYS_EX_ERROR_HANDSHAKE);
                        return false;

                    }

                }

            return true;

    }

    return false;

}

bool OpenDeck::sysExCheckID(uint8_t firstByte, uint8_t secondByte, uint8_t thirdByte)   {

    return  (

    (firstByte  == SYS_EX_M_ID_0)   &&
    (secondByte == SYS_EX_M_ID_1)   &&
    (thirdByte  == SYS_EX_M_ID_2)

    );

}

bool OpenDeck::sysExCheckWish(uint8_t wish)   {

    return ((wish >= SYS_EX_WISH_START) && (wish < SYS_EX_WISH_END));

}

bool OpenDeck::sysExCheckAmount(uint8_t amount)    {

    return ((amount >= SYS_EX_AMOUNT_START) && (amount < SYS_EX_AMOUNT_END));

}

bool OpenDeck::sysExCheckMessageType(uint8_t messageType) {

    return ((messageType >= SYS_EX_MT_START) && (messageType < SYS_EX_MT_END));

}

bool OpenDeck::sysExCheckMessageSubType(uint8_t messageType, uint8_t messageSubType)    {

    switch (messageType)    {

        case SYS_EX_MT_HW_CONFIG:
        return (messageSubType == 0);
        break;

        case SYS_EX_MT_FEATURES:
        return ((messageSubType >= SYS_EX_MST_FEATURES_START) && (messageSubType < SYS_EX_MST_FEATURES_END));
        break;

        case SYS_EX_MT_BUTTON:
        return ((messageSubType >= SYS_EX_MST_BUTTON_START) && (messageSubType < SYS_EX_MST_BUTTON_END));
        break;

        case SYS_EX_MT_POT:
        return ((messageSubType >= SYS_EX_MST_POT_START) && (messageSubType < SYS_EX_MST_POT_END));
        break;

        case SYS_EX_MT_LED:
        return ((messageSubType >= SYS_EX_MST_LED_START) && (messageSubType < SYS_EX_LED_END));
        break;

        case SYS_EX_MT_ALL:
        return (messageSubType == 0);
        break;

        default:
        return false;
        break;

    }

}

bool OpenDeck::sysExCheckParameterID(uint8_t messageType, uint8_t messageSubType, uint8_t parameter)   {

    switch (messageType)    {

        case SYS_EX_MT_HW_CONFIG:
        return ((parameter >= SYS_EX_MST_HW_CONFIG_START) && (parameter < SYS_EX_MST_HW_CONFIG_END));
        break;

        case SYS_EX_MT_FEATURES:
        switch (messageSubType) {

            case SYS_EX_MST_FEATURES_MIDI:
            return ((parameter >= SYS_EX_FEATURES_MIDI_START) && (parameter < SYS_EX_FEATURES_MIDI_END));
            break;

            case SYS_EX_MST_FEATURES_BUTTONS:
            return ((parameter >= SYS_EX_FEATURES_BUTTONS_START) && (parameter < SYS_EX_FEATURES_BUTTONS_END));
            break;

            case SYS_EX_MST_FEATURES_LEDS:
            return ((parameter >= SYS_EX_FEATURES_LEDS_START) && (parameter < SYS_EX_FEATURES_LEDS_END));
            break;

            case SYS_EX_MST_FEATURES_POTS:
            return ((parameter >= SYS_EX_FEATURES_POTS_START) && (parameter < SYS_EX_FEATURES_POTS_END));
            break;

            default:
            return false;
            break;

        }

        case SYS_EX_MT_MIDI_CHANNEL:
        return ((parameter >= SYS_EX_MC_START) && (parameter < SYS_EX_MC_END));
        break;

        case SYS_EX_MT_BUTTON:
        switch (messageSubType) {

            case SYS_EX_MST_BUTTON_TYPE:
            case SYS_EX_MST_BUTTON_PP_ENABLED:
            case SYS_EX_MST_BUTTON_NOTE:
            return (parameter < MAX_NUMBER_OF_BUTTONS);
            break;

            case SYS_EX_MST_BUTTON_HW_P:
            return ((parameter >= SYS_EX_BUTTON_HW_P_START) && (parameter < SYS_EX_MST_BUTTON_HW_P_END));
            break;

            default:
            return false;
            break;

        }

        case SYS_EX_MT_POT:
        return  (parameter < MAX_NUMBER_OF_POTS);
        break;

        case SYS_EX_MT_LED:
        switch (messageSubType) {

            case SYS_EX_MST_LED_ACT_NOTE:
            case SYS_EX_MST_LED_START_UP_NUMBER:
            case SYS_EX_MST_LED_STATE:
            return  (parameter < MAX_NUMBER_OF_LEDS);
            break;

            case SYS_EX_MST_LED_HW_P:
            return ((parameter >= SYS_EX_LED_HW_P_START) && (parameter < SYS_EX_LED_HW_P_END));
            break;

            default:
            return false;
            break;

        }

        break;

        default:
        return false;
        break;

    }

    return false;

}

bool OpenDeck::sysExCheckNewParameterID(uint8_t messageType, uint8_t messageSubType, uint8_t parameter, uint8_t newParameter) {

    switch (messageType)    {

        case SYS_EX_MT_HW_CONFIG:
        switch(parameter)   {

            case SYS_EX_MST_HW_CONFIG_BOARD:
            return ((newParameter >= SYS_EX_BOARD_TYPE_START) && (newParameter < SYS_EX_BOARD_TYPE_END));
            break;

            case SYS_EX_MST_HW_CONFIG_BUTTONS:
            case SYS_EX_MST_HW_CONFIG_LEDS:
            case SYS_EX_MST_HW_CONFIG_POTS:
            return ((newParameter == SYS_EX_ENABLE) || (newParameter == SYS_EX_DISABLE));
            break;

            default:
            return false;
            break;

        }

        break;

        case SYS_EX_MT_FEATURES:
        return ((newParameter == SYS_EX_ENABLE) || (newParameter == SYS_EX_DISABLE));
        break;

        case SYS_EX_MT_MIDI_CHANNEL:
        return ((newParameter >= 1) && (newParameter <= 16));   //there are only 16 MIDI channels
        break;

        case SYS_EX_MT_BUTTON:
        switch (messageSubType) {

            case SYS_EX_MST_BUTTON_TYPE:
            case SYS_EX_MST_BUTTON_PP_ENABLED:
            return ((newParameter == SYS_EX_ENABLE) || (newParameter == SYS_EX_DISABLE));
            break;

            case SYS_EX_MST_BUTTON_NOTE:
            return (newParameter < 128);
            break;

            case SYS_EX_MST_BUTTON_HW_P:
            switch (parameter)  {

                case SYS_EX_BUTTON_HW_P_LONG_PRESS_TIME:
                return ((newParameter >= SYS_EX_BUTTON_LONG_PRESS_TIME_MIN) && (newParameter <= SYS_EX_BUTTON_LONG_PRESS_TIME_MAX));
                break;

                default:
                return false;
                break;

            }

            break;

            default:
            return false;
            break;

        }

        case SYS_EX_MT_POT:
        switch (messageSubType) {

            case SYS_EX_MST_POT_ENABLED:
            case SYS_EX_MST_POT_INVERTED:
            case SYS_EX_MST_POT_PP_ENABLED:
            return ((newParameter == SYS_EX_ENABLE) || (newParameter == SYS_EX_DISABLE));
            break;

            case SYS_EX_MST_POT_CC_PP_NUMBER:
            case SYS_EX_MST_POT_LOWER_LIMIT:
            case SYS_EX_MST_POT_UPPER_LIMIT:
            return (newParameter < 128);
            break;

            default:
            return false;
            break;

        }

        case SYS_EX_MT_LED:
        switch (messageSubType)  {

            case SYS_EX_MST_LED_ACT_NOTE:
            return (newParameter < 128);
            break;

            case SYS_EX_MST_LED_START_UP_NUMBER:
            return (newParameter < totalNumberOfLEDs);
            break;

            case SYS_EX_MST_LED_STATE:
            return ((newParameter >= SYS_EX_LED_STATE_START)  && (newParameter < SYS_EX_LED_STATE_END));
            break;

            case SYS_EX_MST_LED_HW_P:
            switch (parameter)  {

                case SYS_EX_LED_HW_P_TOTAL_NUMBER:
                return (newParameter < MAX_NUMBER_OF_LEDS);
                break;

                case SYS_EX_LED_HW_P_BLINK_TIME:
                return ((newParameter >= SYS_EX_LED_BLINK_TIME_MIN) && (newParameter <= SYS_EX_LED_BLINK_TIME_MAX));
                break;

                case SYS_EX_LED_HW_P_START_UP_SWITCH_TIME:
                return ((newParameter >= SYS_EX_LED_START_UP_SWITCH_TIME_MIN) && (newParameter <= SYS_EX_LED_START_UP_SWITCH_TIME_MAX));
                break;

                case SYS_EX_LED_HW_P_START_UP_ROUTINE:
                return (newParameter < NUMBER_OF_START_UP_ROUTINES);
                break;

            }

            break;

            default:
            return false;
            break;

        }

        break;

        default:
        return false;
        break;

    }

    return false;
}

bool OpenDeck::sysExCheckSpecial(uint8_t messageType, uint8_t wish, uint8_t amount)  {

    //check for restricted combinations in sysex message

    if (messageType == SYS_EX_MT_ALL)   {

        if (wish != SYS_EX_WISH_RESTORE)    {

            //message type SYS_EX_MT_ALL can only be used with SYS_EX_WISH_RESTORE wish
            sysExGenerateError(SYS_EX_ERROR_WISH);
            return false;

        }

        //only ALL amount is allowed for SYS_EX_MT_ALL message type
        if (amount != SYS_EX_AMOUNT_ALL)   {

            sysExGenerateError(SYS_EX_ERROR_AMOUNT);
            return false;

        }

    }

    return true;

}

uint8_t OpenDeck::sysExGenerateMinMessageLenght(uint8_t wish, uint8_t amount, uint8_t messageType, uint8_t messageSubType)    {

    //single parameter
    if (amount == SYS_EX_AMOUNT_SINGLE)  {

        if ((wish == SYS_EX_WISH_GET) ||
        (wish == SYS_EX_WISH_RESTORE))          return SYS_EX_ML_REQ_STANDARD + 1;  //get   //add 1 to length for parameter
        else                                    return SYS_EX_ML_REQ_STANDARD + 2;  //set   //add 2 to length for parameter and new value

        }   else if (amount == SYS_EX_AMOUNT_ALL)   {

        if ((wish == SYS_EX_WISH_GET) || (wish == SYS_EX_WISH_RESTORE))             //get/restore
            return SYS_EX_ML_REQ_STANDARD;

        else    {                                                                   //set

            switch (messageType)    {

                case SYS_EX_MT_HW_CONFIG:
                return SYS_EX_ML_REQ_STANDARD + SYS_EX_MST_HW_CONFIG_END;
                break;

                case SYS_EX_MT_FEATURES:
                switch (messageSubType) {

                    case SYS_EX_MST_FEATURES_MIDI:
                    return SYS_EX_ML_REQ_STANDARD + SYS_EX_FEATURES_MIDI_END;
                    break;

                    case SYS_EX_MST_FEATURES_BUTTONS:
                    return SYS_EX_ML_REQ_STANDARD + SYS_EX_FEATURES_BUTTONS_END;
                    break;

                    case SYS_EX_MST_FEATURES_LEDS:
                    return SYS_EX_ML_REQ_STANDARD + SYS_EX_FEATURES_LEDS_END;
                    break;

                    case SYS_EX_MST_FEATURES_POTS:
                    return SYS_EX_ML_REQ_STANDARD + SYS_EX_FEATURES_POTS_END;
                    break;

                    default:
                    return 0;
                    break;

                }

                case SYS_EX_MT_MIDI_CHANNEL:
                return SYS_EX_ML_REQ_STANDARD + SYS_EX_MC_END;
                break;

                case SYS_EX_MT_BUTTON:
                switch (messageSubType) {

                    case SYS_EX_MST_BUTTON_TYPE:
                    case SYS_EX_MST_BUTTON_PP_ENABLED:
                    case SYS_EX_MST_BUTTON_NOTE:
                    return SYS_EX_ML_REQ_STANDARD + MAX_NUMBER_OF_BUTTONS;
                    break;

                    case SYS_EX_MST_BUTTON_HW_P:
                    return SYS_EX_ML_REQ_STANDARD + SYS_EX_MST_BUTTON_HW_P_END;
                    break;

                    default:
                    return 0;
                    break;

                }

                case SYS_EX_MT_POT:
                return SYS_EX_ML_REQ_STANDARD + MAX_NUMBER_OF_POTS;
                break;

                case SYS_EX_MT_LED:
                switch (messageSubType) {

                    case SYS_EX_MST_LED_ACT_NOTE:
                    case SYS_EX_MST_LED_START_UP_NUMBER:
                    case SYS_EX_MST_LED_STATE:
                    return SYS_EX_ML_REQ_STANDARD + MAX_NUMBER_OF_LEDS;
                    break;

                    case SYS_EX_MST_LED_HW_P:
                    return SYS_EX_LED_HW_P_END;
                    break;

                    default:
                    return 0;
                    break;

                }

                default:
                return 0;
                break;

            }

        }

    }   else return 0;

}

void OpenDeck::sysExGenerateError(uint8_t errorNumber)  {

    uint8_t sysExResponse[5];

    sysExResponse[0] = SYS_EX_M_ID_0;
    sysExResponse[1] = SYS_EX_M_ID_1;
    sysExResponse[2] = SYS_EX_M_ID_2;
    sysExResponse[3] = SYS_EX_ERROR;
    sysExResponse[4] = errorNumber;

    sendSysExDataCallback(sysExResponse, 5);

}

void OpenDeck::sysExGenerateAck()   {

    uint8_t sysExAckResponse[4];

    sysExAckResponse[0] = SYS_EX_M_ID_0;
    sysExAckResponse[1] = SYS_EX_M_ID_1;
    sysExAckResponse[2] = SYS_EX_M_ID_2;
    sysExAckResponse[3] = SYS_EX_ACK;

    sysExEnabled = true;

    sendSysExDataCallback(sysExAckResponse, 4);

}

void OpenDeck::sysExGenerateResponse(uint8_t sysExArray[], uint8_t arrSize)  {

    if (arrSize == SYS_EX_ML_REQ_HANDSHAKE) {

        sysExGenerateAck();
        return;

    }

    uint8_t componentNr     = 1,
            _parameter      = 0;

    int16_t maxComponentNr  = 0;

    //create basic response
    uint8_t sysExResponse[128+SYS_EX_ML_RES_BASIC];

    //copy first part of request to response
    for (int i=0; i<(SYS_EX_ML_RES_BASIC-1); i++)
        sysExResponse[i] = sysExArray[i+1];

    sysExResponse[SYS_EX_ML_RES_BASIC-1] = SYS_EX_ACK;

    switch (sysExArray[SYS_EX_MS_MT])   {

        case SYS_EX_MT_HW_CONFIG:
        maxComponentNr = SYS_EX_MST_HW_CONFIG_END;
        break;

        case SYS_EX_MT_FEATURES:
        switch (sysExArray[SYS_EX_MS_MST])  {

            case SYS_EX_MST_FEATURES_MIDI:
            maxComponentNr = SYS_EX_FEATURES_MIDI_END;
            break;

            case SYS_EX_MST_FEATURES_BUTTONS:
            maxComponentNr = SYS_EX_FEATURES_BUTTONS_END;
            break;

            case SYS_EX_MST_FEATURES_LEDS:
            maxComponentNr = SYS_EX_FEATURES_LEDS_END;
            break;

            case SYS_EX_MST_FEATURES_POTS:
            maxComponentNr = SYS_EX_FEATURES_POTS_END;
            break;

        }

        break;

        case SYS_EX_MT_MIDI_CHANNEL:
        maxComponentNr = SYS_EX_MC_END;
        break;

        case SYS_EX_MT_BUTTON:
        switch (sysExArray[SYS_EX_MS_MST])  {

            case SYS_EX_MST_BUTTON_TYPE:
            case SYS_EX_MST_BUTTON_PP_ENABLED:
            case SYS_EX_MST_BUTTON_NOTE:
            maxComponentNr = MAX_NUMBER_OF_BUTTONS;
            break;

            case SYS_EX_MST_BUTTON_HW_P:
            maxComponentNr = SYS_EX_MST_BUTTON_HW_P_END;
            break;

            default:
            break;

        }

        break;

        case SYS_EX_MT_POT:
        maxComponentNr = MAX_NUMBER_OF_POTS;
        break;

        case SYS_EX_MT_LED:
        switch (sysExArray[SYS_EX_MS_MST])  {

            case SYS_EX_MST_LED_ACT_NOTE:
            case SYS_EX_MST_LED_START_UP_NUMBER:
            maxComponentNr = MAX_NUMBER_OF_LEDS;
            break;

            case SYS_EX_MST_LED_HW_P:
            maxComponentNr = SYS_EX_LED_HW_P_END;
            break;

            default:
            break;

        }

        break;

        case SYS_EX_MT_ALL:
        maxComponentNr = (int16_t)sizeof(defConf);
        break;

        default:
        break;

    }

    if (sysExArray[SYS_EX_MS_AMOUNT] == SYS_EX_AMOUNT_ALL) componentNr = maxComponentNr;

    //create response based on wanted message type
    if (sysExArray[SYS_EX_MS_WISH] == SYS_EX_WISH_GET)    {                     //get

        if (sysExArray[SYS_EX_MS_AMOUNT] == SYS_EX_AMOUNT_ALL)  _parameter = 0;
        else                                                    _parameter = sysExArray[SYS_EX_MS_PARAMETER_ID];

        for (int i=0; i<componentNr; i++) {

            sysExResponse[i+SYS_EX_ML_RES_BASIC] = sysExGet(sysExArray[SYS_EX_MS_MT], sysExArray[SYS_EX_MS_MST], _parameter);
            _parameter++;

        }

        sendSysExDataCallback(sysExResponse, SYS_EX_ML_RES_BASIC+componentNr);
        return;

    }   else    if (sysExArray[SYS_EX_MS_WISH] == SYS_EX_WISH_SET)   {          //set

            uint8_t arrayIndex;

            if (sysExArray[SYS_EX_MS_AMOUNT] == SYS_EX_AMOUNT_ALL) {

                _parameter = 0;
                arrayIndex = SYS_EX_MS_NEW_PARAMETER_ID_ALL;

            }   else    {

                    _parameter = sysExArray[SYS_EX_MS_PARAMETER_ID];
                    arrayIndex = SYS_EX_MS_NEW_PARAMETER_ID_SINGLE;

                }

            for (int i=0; i<componentNr; i++)   {

                if (!sysExSet(sysExArray[SYS_EX_MS_MT], sysExArray[SYS_EX_MS_MST], _parameter, sysExArray[arrayIndex+i]))  {

                    sysExGenerateError(SYS_EX_ERROR_EEPROM);
                    return;

                }

                _parameter++;

            }

            sendSysExDataCallback(sysExResponse, SYS_EX_ML_RES_BASIC);
            return;

        }   else if (sysExArray[SYS_EX_MS_WISH] == SYS_EX_WISH_RESTORE) {       //restore

                if (!sysExRestore(sysExArray[SYS_EX_MS_MT], sysExArray[SYS_EX_MS_MST], sysExArray[SYS_EX_MS_PARAMETER_ID], componentNr)) {

                    sysExGenerateError(SYS_EX_ERROR_EEPROM);
                    return;

                }

                sendSysExDataCallback(sysExResponse, SYS_EX_ML_RES_BASIC);
                return;

            }
}

uint8_t OpenDeck::sysExGet(uint8_t messageType, uint8_t messageSubType, uint8_t parameter)  {

    switch (messageType)    {

        case SYS_EX_MT_HW_CONFIG:
        return sysExGetHardwareConfig(parameter);
        break;

        case SYS_EX_MT_FEATURES:
        return sysExGetFeature(messageSubType, parameter);
        break;

        case SYS_EX_MT_MIDI_CHANNEL:
        return sysExGetMIDIchannel(parameter);
        break;

        case SYS_EX_MT_BUTTON:
        switch (messageSubType) {

            case SYS_EX_MST_BUTTON_TYPE:
            return getButtonType(parameter);
            break;

            case SYS_EX_MST_BUTTON_PP_ENABLED:
            return getButtonPPenabled(parameter);
            break;

            case SYS_EX_MST_BUTTON_NOTE:
            return buttonNote[parameter];
            break;

            case SYS_EX_MST_BUTTON_HW_P:
            return sysExGetButtonHwParameter(parameter);
            break;

            default:
            return false;
            break;

        }

        case SYS_EX_MT_POT:

        switch (messageSubType) {

            case SYS_EX_MST_POT_ENABLED:
            return getPotEnabled(parameter);
            break;

            case SYS_EX_MST_POT_PP_ENABLED:
            return getPotPPenabled(parameter);
            break;

            case SYS_EX_MST_POT_INVERTED:
            return getPotInvertState(parameter);
            break;

            case SYS_EX_MST_POT_CC_PP_NUMBER:
            return ccppNumber[parameter];
            break;

            case SYS_EX_MST_POT_LOWER_LIMIT:
            return ccLowerLimit[parameter];
            break;

            case SYS_EX_MST_POT_UPPER_LIMIT:
            return ccUpperLimit[parameter];
            break;

            default:
            return false;
            break;

        }

        case SYS_EX_MT_LED:
        switch (messageSubType) {

            case SYS_EX_MST_LED_ACT_NOTE:
            return ledActNote[parameter];
            break;

            case SYS_EX_MST_LED_START_UP_NUMBER:
            return eeprom_read_byte((uint8_t*)EEPROM_LED_START_UP_NUMBER_START+parameter);
            break;

            case SYS_EX_MST_LED_STATE:
            return ledState[parameter];
            break;

            case SYS_EX_MST_LED_HW_P:
            return sysExGetLEDHwParameter(parameter);
            break;

            default:
            return false;
            break;

        }

        default:
        return 0;
        break;

    }

}

uint8_t OpenDeck::sysExGetHardwareConfig(uint8_t parameter) {

    switch(parameter)   {

        case SYS_EX_MST_HW_CONFIG_BOARD:
        return _board;
        break;

        case SYS_EX_MST_HW_CONFIG_BUTTONS:
        return bitRead(hardwareEnabled, SYS_EX_MST_HW_CONFIG_BUTTONS);
        break;

        case SYS_EX_MST_HW_CONFIG_LEDS:
        return bitRead(hardwareEnabled, SYS_EX_MST_HW_CONFIG_LEDS);
        break;

        case SYS_EX_MST_HW_CONFIG_POTS:
        return bitRead(hardwareEnabled, SYS_EX_MST_HW_CONFIG_POTS);
        break;

        default:
        return 0;
        break;

    }

}

bool OpenDeck::sysExGetFeature(uint8_t featureType, uint8_t feature)    {

    switch (featureType)    {

        case SYS_EX_MST_FEATURES_MIDI:
        return bitRead(midiFeatures, feature);
        break;

        case SYS_EX_MST_FEATURES_BUTTONS:
        return bitRead(buttonFeatures, feature);
        break;

        case SYS_EX_MST_FEATURES_LEDS:
        return bitRead(ledFeatures, feature);
        break;

        case SYS_EX_MST_FEATURES_POTS:
        return bitRead(potFeatures, feature);
        break;

        default:
        return false;
        break;

    }

    return false;

}

uint8_t OpenDeck::sysExGetMIDIchannel(uint8_t channel)  {

    switch (channel)    {

        case SYS_EX_MC_BUTTON_NOTE:
        return _buttonNoteChannel;
        break;

        case SYS_EX_MC_LONG_PRESS_BUTTON_NOTE:
        return _longPressButtonNoteChannel;
        break;

        case SYS_EX_MC_BUTTON_PP:
        return _buttonPPchannel;
        break;

        case SYS_EX_MC_POT_CC:
        return _potCCchannel;
        break;

        case SYS_EX_MC_POT_PP:
        return _potPPchannel;
        break;

        case SYS_EX_MC_POT_NOTE:
        return _potNoteChannel;
        break;

        case SYS_EX_MC_INPUT:
        return _inputChannel;
        break;

        default:
        return 0;
        break;

    }

}

uint8_t OpenDeck::sysExGetButtonHwParameter(uint8_t parameter)    {

    switch(parameter)   {

        case SYS_EX_BUTTON_HW_P_LONG_PRESS_TIME:
        return eeprom_read_byte((uint8_t*)EEPROM_BUTTON_HW_P_LONG_PRESS_TIME);
        break;

        default:
        return 0;
        break;

    }

}

uint8_t OpenDeck::sysExGetLEDHwParameter(uint8_t parameter)  {

    switch (parameter)  {

        case SYS_EX_LED_HW_P_TOTAL_NUMBER:
        return totalNumberOfLEDs;
        break;

        case SYS_EX_LED_HW_P_BLINK_TIME:
        return eeprom_read_byte((uint8_t*)EEPROM_LED_HW_P_BLINK_TIME);
        break;

        case SYS_EX_LED_HW_P_START_UP_SWITCH_TIME:
        return eeprom_read_byte((uint8_t*)EEPROM_LED_HW_P_START_UP_SWITCH_TIME);
        break;

        case SYS_EX_LED_HW_P_START_UP_ROUTINE:
        return eeprom_read_byte((uint8_t*)EEPROM_LED_HW_P_START_UP_ROUTINE);
        break;

        default:
        return 0;
        break;

    }

}

bool OpenDeck::sysExSet(uint8_t messageType, uint8_t messageSubType, uint8_t parameter, uint8_t newParameter)    {

    switch (messageType)    {

        case SYS_EX_MT_HW_CONFIG:
        return sysExSetHardwareConfig(parameter, newParameter);
        break;

        case SYS_EX_MT_MIDI_CHANNEL:
        return sysExSetMIDIchannel(parameter, newParameter);
        break;

        case SYS_EX_MT_FEATURES:
        return sysExSetFeature(messageType, parameter, newParameter);
        break;

        case SYS_EX_MT_BUTTON:
        switch (messageSubType) {

            case SYS_EX_MST_BUTTON_TYPE:
            return sysExSetButtonType(parameter, newParameter);
            break;

            case SYS_EX_MST_BUTTON_PP_ENABLED:
            return sysExSetButtonPPenabled(parameter, newParameter);
            break;

            case SYS_EX_MST_BUTTON_NOTE:
            return sysExSetButtonNote(parameter, newParameter);
            break;

            case SYS_EX_MST_BUTTON_HW_P:
            return sysExSetButtonHwParameter(parameter, newParameter);
            break;

            default:
            return false;
            break;

        }

        break;

        case SYS_EX_MT_POT:

        switch (messageSubType) {

            case SYS_EX_MST_POT_ENABLED:
            return sysExSetPotEnabled(parameter, newParameter);
            break;

            case SYS_EX_MST_POT_PP_ENABLED:
            return sysExSetPotPPEnabled(parameter, newParameter);
            break;

            case SYS_EX_MST_POT_INVERTED:
            return sysExSetPotInvertState(parameter, newParameter);
            break;

            case SYS_EX_MST_POT_CC_PP_NUMBER:
            return sysExSetCCPPnumber(parameter, newParameter);
            break;

            case SYS_EX_MST_POT_LOWER_LIMIT:
            case SYS_EX_MST_POT_UPPER_LIMIT:
            return sysExSetCClimit(messageSubType, parameter, newParameter);
            break;

            default:
            return false;
            break;

        }

        case SYS_EX_MT_LED:
        switch(messageSubType)  {

            case SYS_EX_MST_LED_ACT_NOTE:
            return sysExSetLEDnote(parameter, newParameter);
            break;

            case SYS_EX_MST_LED_START_UP_NUMBER:
            return sysExSetLEDstartNumber(parameter, newParameter);
            break;

            case SYS_EX_MST_LED_HW_P:
            return sysExSetLEDHwParameter(parameter, newParameter);
            break;

            case SYS_EX_MST_LED_STATE:
            switch (newParameter)   {

                case SYS_EX_LED_STATE_C_OFF:
                handleLED(false, false, parameter);
                return true;
                break;

                case SYS_EX_LED_STATE_C_ON:
                handleLED(true, false, parameter);
                return true;
                break;

                case SYS_EX_LED_STATE_B_OFF:
                handleLED(false, true, parameter);
                return true;
                break;

                case SYS_EX_LED_STATE_B_ON:
                handleLED(true, true, parameter);
                return true;
                break;

                default:
                return false;
                break;

            }

        }

        break;

        default:
        return false;
        break;

    }

    return false;

}

bool OpenDeck::sysExRestore(uint8_t messageType, uint8_t messageSubType, uint16_t parameter, int16_t componentNr) {

    uint16_t    eepromAddress = 0,
                _parameter    = 0;

    switch (messageType)    {

        case SYS_EX_MT_HW_CONFIG:
        eepromAddress = EEPROM_HW_CONFIG_START;
        break;

        case SYS_EX_MT_FEATURES:
        switch (messageSubType) {

            case SYS_EX_MST_FEATURES_MIDI:
            eepromAddress = EEPROM_FEATURES_MIDI;
            break;

            case SYS_EX_MST_FEATURES_BUTTONS:
            eepromAddress = EEPROM_FEATURES_BUTTONS;
            break;

            case SYS_EX_MST_FEATURES_LEDS:
            eepromAddress = EEPROM_FEATURES_LEDS;
            break;

            case SYS_EX_MST_FEATURES_POTS:
            eepromAddress = EEPROM_FEATURES_POTS;
            break;

            default:
            return false;
            break;

        }

        break;

        case SYS_EX_MT_MIDI_CHANNEL:
        eepromAddress = EEPROM_MC_START;
        break;

        case SYS_EX_MT_BUTTON:
        switch (messageSubType) {

            case SYS_EX_MST_BUTTON_TYPE:
            eepromAddress = EEPROM_BUTTON_TYPE_START;
            break;

            case SYS_EX_MST_BUTTON_PP_ENABLED:
            eepromAddress = EEPROM_BUTTON_PP_ENABLED_START;
            break;

            case SYS_EX_MST_BUTTON_NOTE:
            eepromAddress = EEPROM_BUTTON_NOTE_START;
            break;

            case SYS_EX_MST_BUTTON_HW_P:
            eepromAddress = EEPROM_BUTTON_HW_P_START;
            break;

            default:
            return false;
            break;

        }

        break;

        case SYS_EX_MT_POT:
        switch (messageSubType) {

            case SYS_EX_MST_POT_ENABLED:
            eepromAddress = EEPROM_POT_ENABLED_START;
            break;

            case SYS_EX_MST_POT_PP_ENABLED:
            eepromAddress = EEPROM_POT_PP_ENABLED_START;
            break;

            case SYS_EX_MST_POT_INVERTED:
            eepromAddress = EEPROM_POT_INVERSION_START;
            break;

            case SYS_EX_MST_POT_CC_PP_NUMBER:
            eepromAddress = EEPROM_POT_CC_PP_NUMBER_START;
            break;

            case SYS_EX_MST_POT_LOWER_LIMIT:
            eepromAddress = EEPROM_POT_LOWER_LIMIT_START;
            break;

            case SYS_EX_MST_POT_UPPER_LIMIT:
            eepromAddress = EEPROM_POT_UPPER_LIMIT_START;
            break;

            default:
            return false;
            break;

        }

        break;

        case SYS_EX_MT_LED:
        switch (messageSubType) {

            case SYS_EX_MST_LED_ACT_NOTE:
            eepromAddress = EEPROM_LED_ACT_NOTE_START;
            break;

            case SYS_EX_MST_LED_START_UP_NUMBER:
            eepromAddress = EEPROM_LED_START_UP_NUMBER_START;
            break;

            case SYS_EX_MST_LED_HW_P:
            eepromAddress = EEPROM_LED_HW_P_START;
            break;

            default:
            return false;
            break;

        }

        break;

        case SYS_EX_MT_ALL:
        eepromAddress = 0;
        break;

        default:
        return false;
        break;

    }

    if (messageSubType == SYS_EX_MST_LED_STATE) {

        for (int i=0; i<MAX_NUMBER_OF_LEDS; i++)    ledState[i] = 0;
        return true;

    }   else if (messageType == SYS_EX_MT_ALL)  {

            return sysExSetDefaultConf();

        }   else    {

                if (componentNr == 1) _parameter = parameter;

                for (int i=0; i<componentNr; i++)    {

                    if ((!sysExSet(messageType, messageSubType, _parameter, pgm_read_byte(&(defConf[eepromAddress+_parameter])))) && 
                        (!((eeprom_read_byte((uint8_t*)eepromAddress+i)) == (pgm_read_byte(&defConf[eepromAddress+i]))))) return false;

                        _parameter++;

                }

                return true;

            }

}

bool OpenDeck::sysExSetMIDIchannel(uint8_t channel, uint8_t channelNumber)  {

    switch (channel)    {

        case SYS_EX_MC_BUTTON_NOTE:
        _buttonNoteChannel = channelNumber;
        eeprom_update_byte((uint8_t*)EEPROM_MC_BUTTON_NOTE, channelNumber);
        return (channelNumber == eeprom_read_byte((uint8_t*)EEPROM_MC_BUTTON_NOTE));
        break;

        case SYS_EX_MC_LONG_PRESS_BUTTON_NOTE:
        _longPressButtonNoteChannel = channelNumber;
        eeprom_update_byte((uint8_t*)EEPROM_MC_LONG_PRESS_BUTTON_NOTE, channelNumber);
        return (channelNumber == eeprom_read_byte((uint8_t*)EEPROM_MC_LONG_PRESS_BUTTON_NOTE));
        break;

        case SYS_EX_MC_POT_CC:
        _potCCchannel = channelNumber;
        eeprom_update_byte((uint8_t*)EEPROM_MC_POT_CC, channelNumber);
        return (channelNumber == eeprom_read_byte((uint8_t*)EEPROM_MC_POT_CC));
        break;

        case SYS_EX_MC_POT_NOTE:
        _potNoteChannel = channelNumber;
        eeprom_update_byte((uint8_t*)EEPROM_MC_POT_NOTE, channelNumber);
        return (channelNumber == eeprom_read_byte((uint8_t*)EEPROM_MC_POT_NOTE));
        break;

        case SYS_EX_MC_INPUT:
        _inputChannel = channelNumber;
        eeprom_update_byte((uint8_t*)EEPROM_MC_INPUT, channelNumber);
        return (channelNumber == eeprom_read_byte((uint8_t*)EEPROM_MC_INPUT));
        break;

        default:
        return false;

    }

}

bool OpenDeck::sysExSetHardwareConfig(uint8_t parameter, uint8_t value)  {

    switch (parameter)  {

        case SYS_EX_MST_HW_CONFIG_BOARD:
        _board = value;
        eeprom_update_byte((uint8_t*)EEPROM_BOARD_TYPE, value);
        if (eeprom_read_byte((uint8_t*)EEPROM_BOARD_TYPE) == value) { initBoard(); return true; }
        break;

        case SYS_EX_MST_HW_CONFIG_BUTTONS:
        case SYS_EX_MST_HW_CONFIG_LEDS:
        case SYS_EX_MST_HW_CONFIG_POTS:
        //enable/disable specified hardware
        bitWrite(hardwareEnabled, parameter, value);
        eeprom_update_byte((uint8_t*)EEPROM_HARDWARE_ENABLED, hardwareEnabled);
        return (eeprom_read_byte((uint8_t*)EEPROM_HARDWARE_ENABLED) == hardwareEnabled);
        break;

        default:
        return false;
        break;

    }

    return false;

}

bool OpenDeck::sysExSetFeature(uint8_t featureType, uint8_t feature, bool state)    {

    switch (featureType)    {

        case SYS_EX_MST_FEATURES_MIDI:
        //MIDI features
        bitWrite(midiFeatures, feature, state);
        eeprom_update_byte((uint8_t*)EEPROM_FEATURES_MIDI, midiFeatures);
        return (eeprom_read_byte((uint8_t*)EEPROM_FEATURES_MIDI) == midiFeatures);
        break;

        case SYS_EX_MST_FEATURES_BUTTONS:
        //button features
        bitWrite(buttonFeatures, feature, state);
        eeprom_update_byte((uint8_t*)EEPROM_FEATURES_BUTTONS, buttonFeatures);
        return (eeprom_read_byte((uint8_t*)EEPROM_FEATURES_BUTTONS) == buttonFeatures);
        break;

        case SYS_EX_MST_FEATURES_LEDS:
        //LED features
        bitWrite(ledFeatures, feature, state);
        if ((feature == SYS_EX_FEATURES_LEDS_BLINK) && (state == SYS_EX_DISABLE))
        for (int i=0; i<MAX_NUMBER_OF_LEDS; i++) handleLED(false, true, i); //remove all blinking bits from ledState
        eeprom_update_byte((uint8_t*)EEPROM_FEATURES_LEDS, ledFeatures);
        return (eeprom_read_byte((uint8_t*)EEPROM_FEATURES_LEDS) == ledFeatures);
        break;

        case SYS_EX_MST_FEATURES_POTS:
        //pot features
        bitWrite(potFeatures, feature, state);
        eeprom_update_byte((uint8_t*)EEPROM_FEATURES_POTS, potFeatures);
        return (eeprom_read_byte((uint8_t*)EEPROM_FEATURES_POTS) == potFeatures);
        break;

        default:
        break;

    }   return false;

}

bool OpenDeck::sysExSetButtonType(uint8_t buttonNumber, bool type)  {

    uint8_t arrayIndex = buttonNumber/8;
    uint8_t buttonIndex = buttonNumber - 8*arrayIndex;
    uint16_t eepromAddress = EEPROM_BUTTON_TYPE_START+arrayIndex;

    bitWrite(buttonType[arrayIndex], buttonIndex, type);
    eeprom_update_byte((uint8_t*)eepromAddress, buttonType[arrayIndex]);

    resetLongPress(buttonNumber);
    setButtonPressed(buttonNumber, false);
    updateButtonState(buttonNumber, false);

    return (buttonType[arrayIndex] == eeprom_read_byte((uint8_t*)eepromAddress));

}

bool OpenDeck::sysExSetButtonPPenabled(uint8_t buttonNumber, bool value)  {

    uint8_t arrayIndex = buttonNumber/8;
    uint8_t buttonIndex = buttonNumber - 8*arrayIndex;
    uint16_t eepromAddress = EEPROM_BUTTON_PP_ENABLED_START+arrayIndex;

    bitWrite(buttonPPenabled[arrayIndex], buttonIndex, value);
    eeprom_update_byte((uint8_t*)eepromAddress, buttonPPenabled[arrayIndex]);

    resetLongPress(buttonNumber);
    setButtonPressed(buttonNumber, false);
    updateButtonState(buttonNumber, false);

    return (buttonPPenabled[arrayIndex] == eeprom_read_byte((uint8_t*)eepromAddress));

}

bool OpenDeck::sysExSetButtonNote(uint8_t buttonNumber, uint8_t _buttonNote)    {

    uint16_t eepromAddress = EEPROM_BUTTON_NOTE_START+buttonNumber;

    buttonNote[buttonNumber] = _buttonNote;
    eeprom_update_byte((uint8_t*)eepromAddress, _buttonNote);
    return (_buttonNote == eeprom_read_byte((uint8_t*)eepromAddress));

}

bool OpenDeck::sysExSetButtonHwParameter(uint8_t parameter, uint8_t value)   {

    switch (parameter)  {

        case SYS_EX_BUTTON_HW_P_LONG_PRESS_TIME:
        //long press time
        eeprom_update_byte((uint8_t*)EEPROM_BUTTON_HW_P_LONG_PRESS_TIME, value);
        setNumberOfLongPressPasses();
        return (eeprom_read_byte((uint8_t*)EEPROM_BUTTON_HW_P_LONG_PRESS_TIME) == value);
        break;

        default:
        return false;
        break;

    }

}

bool OpenDeck::sysExSetLEDHwParameter(uint8_t parameter, uint8_t value) {

    switch(parameter)   {

        case SYS_EX_LED_HW_P_TOTAL_NUMBER:
        //set total number of LEDs (needed for start-up routine)
        totalNumberOfLEDs = value;
        eeprom_update_byte((uint8_t*)EEPROM_LED_HW_P_TOTAL_NUMBER, value);
        return (eeprom_read_byte((uint8_t*)EEPROM_LED_HW_P_TOTAL_NUMBER) == value);
        break;

        case SYS_EX_LED_HW_P_BLINK_TIME:
        //blink time
        _blinkTime = value*100;
        eeprom_update_byte((uint8_t*)EEPROM_LED_HW_P_BLINK_TIME, value);
        return (eeprom_read_byte((uint8_t*)EEPROM_LED_HW_P_BLINK_TIME) == value);
        break;

        case SYS_EX_LED_HW_P_START_UP_SWITCH_TIME:
        //start-up led switch time
        eeprom_update_byte((uint8_t*)EEPROM_LED_HW_P_START_UP_SWITCH_TIME, value);
        return (eeprom_read_byte((uint8_t*)EEPROM_LED_HW_P_START_UP_SWITCH_TIME) == value);
        break;

        case SYS_EX_LED_HW_P_START_UP_ROUTINE:
        //set start-up routine pattern
        eeprom_update_byte((uint8_t*)EEPROM_LED_HW_P_START_UP_ROUTINE, value);
        return (eeprom_read_byte((uint8_t*)EEPROM_LED_HW_P_START_UP_ROUTINE) == value);
        break;

    }   return false;

}

bool OpenDeck::sysExSetPotEnabled(uint8_t potNumber, bool state)    {

    uint8_t arrayIndex = potNumber/8;
    uint8_t potIndex = potNumber - 8*arrayIndex;
    uint16_t eepromAddress = EEPROM_POT_ENABLED_START+arrayIndex;

    bitWrite(potEnabled[arrayIndex], potIndex, state);
    eeprom_update_byte((uint8_t*)eepromAddress, potEnabled[arrayIndex]);

    return (potEnabled[arrayIndex] == eeprom_read_byte((uint8_t*)eepromAddress));

}

bool OpenDeck::sysExSetPotPPEnabled(uint8_t potNumber, bool state)    {

    uint8_t arrayIndex = potNumber/8;
    uint8_t potIndex = potNumber - 8*arrayIndex;
    uint16_t eepromAddress = EEPROM_POT_PP_ENABLED_START+arrayIndex;

    bitWrite(potPPenabled[arrayIndex], potIndex, state);
    eeprom_update_byte((uint8_t*)eepromAddress, potEnabled[arrayIndex]);

    return (potPPenabled[arrayIndex] == eeprom_read_byte((uint8_t*)eepromAddress));

}

bool OpenDeck::sysExSetPotInvertState(uint8_t potNumber, bool state)    {

    uint8_t arrayIndex = potNumber/8;
    uint8_t potIndex = potNumber - 8*arrayIndex;
    uint16_t eepromAddress = EEPROM_POT_INVERSION_START+arrayIndex;

    bitWrite(potInverted[arrayIndex], potIndex, state);
    eeprom_update_byte((uint8_t*)eepromAddress, potInverted[arrayIndex]);

    return (potInverted[arrayIndex] == eeprom_read_byte((uint8_t*)eepromAddress));

}

bool OpenDeck::sysExSetCCPPnumber(uint8_t potNumber, uint8_t _ccppNumber)   {

    uint16_t eepromAddress = EEPROM_POT_CC_PP_NUMBER_START+potNumber;

    ccppNumber[potNumber] = _ccppNumber;
    eeprom_update_byte((uint8_t*)eepromAddress, _ccppNumber);
    return (_ccppNumber == eeprom_read_byte((uint8_t*)eepromAddress));

}

bool OpenDeck::sysExSetCClimit(uint8_t limitType, uint8_t _ccNumber, uint8_t newLimit)  {

    switch (limitType)  {

        case SYS_EX_MST_POT_LOWER_LIMIT:
        ccLowerLimit[_ccNumber] = newLimit;
        eeprom_update_byte((uint8_t*)EEPROM_POT_LOWER_LIMIT_START+_ccNumber, newLimit);
        return (eeprom_read_byte((uint8_t*)EEPROM_POT_LOWER_LIMIT_START+_ccNumber) == newLimit);
        break;

        case SYS_EX_MST_POT_UPPER_LIMIT:
        ccUpperLimit[_ccNumber] = newLimit;
        eeprom_update_byte((uint8_t*)EEPROM_POT_UPPER_LIMIT_START+_ccNumber, newLimit);
        return (eeprom_read_byte((uint8_t*)EEPROM_POT_UPPER_LIMIT_START+_ccNumber) == newLimit);
        break;

        default:
        return false;
        break;

    }

}

bool OpenDeck::sysExSetLEDnote(uint8_t ledNumber, uint8_t _ledActNote) {

    uint16_t eepromAddress = EEPROM_LED_ACT_NOTE_START+ledNumber;

    ledActNote[ledNumber] = _ledActNote;
    //TODO: add check of same values
    eeprom_update_byte((uint8_t*)eepromAddress, _ledActNote);
    return _ledActNote == eeprom_read_byte((uint8_t*)eepromAddress);

}

bool OpenDeck::sysExSetLEDstartNumber(uint8_t ledNumber, uint8_t startNumber) {

    uint16_t eepromAddress = EEPROM_LED_START_UP_NUMBER_START+startNumber;
    //TODO: add check of same values
    eeprom_update_byte((uint8_t*)eepromAddress, ledNumber);
    return ledNumber == eeprom_read_byte((uint8_t*)eepromAddress);

}

bool OpenDeck::sysExSetDefaultConf()    {

    //write default configuration stored in PROGMEM to EEPROM
    for (int i=0; i<(int16_t)sizeof(defConf); i++)  {

        eeprom_update_byte((uint8_t*)i, pgm_read_byte(&(defConf[i])));
        if (!(eeprom_read_byte((uint8_t*)i) == (pgm_read_byte(&(defConf[i])))))
            return false;

    }

    getConfiguration();
    return true;

}