/*

OpenDECK library v1.1
File: SysEx.cpp
Last revision date: 2014-09-24
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
                        sysExArray[SYS_EX_MS_MT]))    {

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

        case SYS_EX_MT_MIDI_CHANNEL:
        return (messageSubType == 0);
        break;

        case SYS_EX_MT_HW_PARAMETER:
        return (messageSubType == 0);
        break;

        case SYS_EX_MT_FREE_PINS:
        return (messageSubType == 0);
        break;

        case SYS_EX_MT_SW_FEATURE:
        return (messageSubType == 0);
        break;

        case SYS_EX_MT_HW_FEATURE:
        return (messageSubType == 0);
        break;

        case SYS_EX_MT_BUTTON:
        return ((messageSubType >= SYS_EX_MST_BUTTON_START) && (messageSubType < SYS_EX_MST_BUTTON_END));
        break;

        case SYS_EX_MT_POT:
        return ((messageSubType >= SYS_EX_MST_POT_START) && (messageSubType < SYS_EX_MST_POT_END));
        break;

        case SYS_EX_MT_ENC:
        //to-do
        return false;
        break;

        case SYS_EX_MT_LED:
        return ((messageSubType >= SYS_EX_MST_LED_START) && (messageSubType < SYS_EX_MST_LED_END));
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

        case SYS_EX_MT_MIDI_CHANNEL:
        return ((parameter >= SYS_EX_MC_START) && (parameter < SYS_EX_MC_END));
        break;

        case SYS_EX_MT_HW_PARAMETER:
        return ((parameter >= SYS_EX_HW_P_START) && (parameter < SYS_EX_HW_P_END));
        break;

        case SYS_EX_MT_FREE_PINS:
        return ((parameter >= SYS_EX_FREE_PIN_START) && (parameter < SYS_EX_FREE_PIN_END));
        break;

        case SYS_EX_MT_SW_FEATURE:
        return ((parameter >= SYS_EX_SW_F_START) && (parameter < SYS_EX_SW_F_END));
        break;

        case SYS_EX_MT_HW_FEATURE:
        return ((parameter >= SYS_EX_HW_F_START) && (parameter < SYS_EX_HW_F_END));
        break;

        case SYS_EX_MT_BUTTON:
        return  (parameter < MAX_NUMBER_OF_BUTTONS);
        break;

        case SYS_EX_MT_POT:
        return  (parameter < MAX_NUMBER_OF_POTS);
        break;

        case SYS_EX_MT_ENC:
        return  (parameter < (MAX_NUMBER_OF_BUTTONS/2));
        break;

        case SYS_EX_MT_LED:
        return  (parameter < MAX_NUMBER_OF_LEDS);
        break;

        default:
        return false;
        break;

    }

    return false;

}

bool OpenDeck::sysExCheckNewParameterID(uint8_t messageType, uint8_t messageSubType, uint8_t parameter, uint8_t newParameter) {

    switch (messageType)    {

        case SYS_EX_MT_MIDI_CHANNEL:
        return ((newParameter >= 1) && (newParameter <= 16));   //there are only 16 MIDI channels
        break;

        case SYS_EX_MT_HW_PARAMETER:

        switch (parameter)    {

            case SYS_EX_HW_P_BOARD_TYPE:
            return ((newParameter >= SYS_EX_BOARD_TYPE_START) && (newParameter < SYS_EX_BOARD_TYPE_END));
            break;

            case SYS_EX_HW_P_LONG_PRESS_TIME:
            return ((newParameter >= SYS_EX_HW_P_LONG_PRESS_TIME_MIN) && (newParameter <= SYS_EX_HW_P_LONG_PRESS_TIME_MAX));
            break;

            case SYS_EX_HW_P_BLINK_TIME:
            return ((newParameter >= SYS_EX_HW_P_BLINK_TIME_MIN) && (newParameter <= SYS_EX_HW_P_BLINK_TIME_MAX));
            break;

            case SYS_EX_HW_P_START_UP_SWITCH_TIME:
            return ((newParameter >= SYS_EX_HW_P_START_UP_SWITCH_TIME_MIN) && (newParameter <= SYS_EX_HW_P_START_UP_SWITCH_TIME_MAX));
            break;

            case SYS_EX_HW_P_START_UP_ROUTINE:
            return (newParameter < NUMBER_OF_START_UP_ROUTINES);
            break;

            case SYS_EX_HW_P_TOTAL_LED_NUMBER:
            return (newParameter < MAX_NUMBER_OF_LEDS);
            break;

            default:
            return false;
            break;

        }

        break;

        case SYS_EX_MT_FREE_PINS:

        switch (parameter)  {

            case SYS_EX_FREE_PIN_A:
            case SYS_EX_FREE_PIN_B:
            case SYS_EX_FREE_PIN_D:
            return ((newParameter >= SYS_EX_FREE_PIN_STATE_START) && (newParameter < SYS_EX_FREE_PIN_STATE_END));
            break;

            case SYS_EX_FREE_PIN_C:
            return  (

            (newParameter == SYS_EX_FREE_PIN_STATE_DISABLED) ||
            (newParameter == SYS_EX_FREE_PIN_STATE_L_ROW)

            );

            break;

            default:
            return false;
            break;

        }

        break;

        case SYS_EX_MT_SW_FEATURE:
        case SYS_EX_MT_HW_FEATURE:
        return ((newParameter == SYS_EX_ENABLE) || (newParameter == SYS_EX_DISABLE));
        break;

        case SYS_EX_MT_BUTTON:

        switch (messageSubType) {

            case SYS_EX_MST_BUTTON_TYPE:
            return (

                (newParameter == SYS_EX_BUTTON_TYPE_MOMENTARY) ||
                (newParameter == SYS_EX_BUTTON_TYPE_LATCHING)
                
            );

            break;

            case SYS_EX_MST_BUTTON_NOTE:
            return (newParameter < 128);
            break;
            
            default:
            return false;
            break;

        }

        break;

        case SYS_EX_MT_POT:
        switch (messageSubType) {

            case SYS_EX_MST_POT_ENABLED:
            case SYS_EX_MST_POT_INVERTED:
            return ((newParameter == SYS_EX_ENABLE) || (newParameter == SYS_EX_DISABLE));
            break;

            case SYS_EX_MST_POT_CC_NUMBER:
            case SYS_EX_MST_POT_LOWER_LIMIT:
            case SYS_EX_MST_POT_UPPER_LIMIT:
            return (newParameter < 128);
            break;

            default:
            return false;
            break;

        }

        case SYS_EX_MT_ENC:
        return (newParameter < 128);
        break;

        case SYS_EX_MT_LED:

        switch (messageSubType)  {

            case SYS_EX_MST_LED_ACT_NOTE:
            return (newParameter < 128);
            break;

            case SYS_EX_MST_LED_START_UP_NUMBER:
            return (newParameter < MAX_NUMBER_OF_LEDS);

            case SYS_EX_MST_LED_STATE:
            return ((newParameter >= SYS_EX_LED_STATE_START)  && (newParameter < SYS_EX_LED_STATE_END));
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

    //don't allow free pin configuration if the board doesn't support it
    if ((!freePinConfEn) && (messageType == SYS_EX_MT_FREE_PINS))  {

        sysExGenerateError(SYS_EX_ERROR_NOT_SUPPORTED);
        return false;

    }

    if (messageType == SYS_EX_MT_ALL)  {

        //message type SYS_EX_MT_ALL can only be used with SYS_EX_WISH_RESTORE wish
        if ((wish == SYS_EX_WISH_GET) || (wish == SYS_EX_WISH_SET)) {

            sysExGenerateError(SYS_EX_ERROR_WISH);
            return false;

        }

        //only ALL amount is allowed for SYS_EX_MT_ALL message type
        if (amount == SYS_EX_AMOUNT_SINGLE)   {

            sysExGenerateError(SYS_EX_ERROR_AMOUNT);
            return false;

        }

    }

    return true;

}

uint8_t OpenDeck::sysExGenerateMinMessageLenght(uint8_t wish, uint8_t amount, uint8_t messageType)    {

    //single parameter
    if (amount == SYS_EX_AMOUNT_SINGLE)  {

        if ((wish == SYS_EX_WISH_GET) ||
        (wish == SYS_EX_WISH_RESTORE))          return SYS_EX_ML_REQ_STANDARD + 1;  //get   //add 1 to length for parameter
        else                                    return SYS_EX_ML_REQ_STANDARD + 2;  //set   //add 2 to length for parameter and new value

        }   else if (amount == SYS_EX_AMOUNT_ALL)   {

        if ((wish == SYS_EX_WISH_GET) || (wish == SYS_EX_WISH_RESTORE))         //get/restore
            return SYS_EX_ML_REQ_STANDARD;

        else    {                                                               //set

            switch (messageType)    {

                case SYS_EX_MT_MIDI_CHANNEL:
                return SYS_EX_ML_REQ_STANDARD + SYS_EX_MC_END;
                break;

                case SYS_EX_MT_HW_PARAMETER:
                return SYS_EX_ML_REQ_STANDARD + SYS_EX_HW_P_END;
                break;

                case SYS_EX_MT_FREE_PINS:
                return SYS_EX_ML_REQ_STANDARD + SYS_EX_FREE_PIN_END;
                break;

                case SYS_EX_MT_SW_FEATURE:
                return SYS_EX_ML_REQ_STANDARD + SYS_EX_SW_F_END;
                break;

                case SYS_EX_MT_HW_FEATURE:
                return SYS_EX_ML_REQ_STANDARD + SYS_EX_HW_F_END;
                break;

                case SYS_EX_MT_BUTTON:
                return SYS_EX_ML_REQ_STANDARD + MAX_NUMBER_OF_BUTTONS;
                break;
                
                case SYS_EX_MT_POT:
                return SYS_EX_ML_REQ_STANDARD + MAX_NUMBER_OF_POTS;
                break;
                
                case SYS_EX_MT_LED:
                return SYS_EX_ML_REQ_STANDARD + MAX_NUMBER_OF_LEDS;
                break;

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

        case SYS_EX_MT_MIDI_CHANNEL:
        maxComponentNr = SYS_EX_MC_END;
        break;

        case SYS_EX_MT_HW_PARAMETER:
        maxComponentNr = SYS_EX_HW_P_END;
        break;

        case SYS_EX_MT_FREE_PINS:
        maxComponentNr = SYS_EX_FREE_PIN_END;
        break;

        case SYS_EX_MT_SW_FEATURE:
        maxComponentNr = SYS_EX_SW_F_END;
        break;

        case SYS_EX_MT_HW_FEATURE:
        maxComponentNr = SYS_EX_HW_F_END;
        break;

        case SYS_EX_MT_BUTTON:
        maxComponentNr = MAX_NUMBER_OF_BUTTONS;
        break;

        case SYS_EX_MT_POT:
        maxComponentNr = MAX_NUMBER_OF_POTS;
        break;

        case SYS_EX_MT_ENC:
        maxComponentNr = MAX_NUMBER_OF_BUTTONS/2;
        break;

        case SYS_EX_MT_LED:
        maxComponentNr = MAX_NUMBER_OF_LEDS;
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

        case SYS_EX_MT_MIDI_CHANNEL:
        return sysExGetMIDIchannel(parameter);
        break;

        case SYS_EX_MT_HW_PARAMETER:
        return sysExGetHardwareParameter(parameter);
        break;

        case SYS_EX_MT_FREE_PINS:
        return freePinState[parameter];
        break;

        case SYS_EX_MT_SW_FEATURE:
        case SYS_EX_MT_HW_FEATURE:
        return sysExGetFeature(messageType, parameter);
        break;

        case SYS_EX_MT_BUTTON:
        if (messageSubType == SYS_EX_MST_BUTTON_TYPE)                   return getButtonType(parameter);
        else                                                            return buttonNote[parameter];
        break;

        case SYS_EX_MT_POT:

        switch (messageSubType) {

            case SYS_EX_MST_POT_ENABLED:
            return getPotEnabled(parameter);
            break;
            
            case SYS_EX_MST_POT_INVERTED:
            return getPotInvertState(parameter);
            break;
            
            case SYS_EX_MST_POT_CC_NUMBER:
            return ccNumber[parameter];
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
        if (messageSubType == SYS_EX_MST_LED_ACT_NOTE)                  return ledNote[parameter];
        else if (messageSubType == SYS_EX_MST_LED_START_UP_NUMBER)      return eeprom_read_byte((uint8_t*)EEPROM_LED_START_UP_NUMBER_START+parameter);
        else if (messageSubType == SYS_EX_MST_LED_STATE)                return ledState[parameter];
        else return 0;
        break;

        default:
        return 0;
        break;

    }

}

uint8_t OpenDeck::sysExGetMIDIchannel(uint8_t channel)  {

    switch (channel)    {

        case SYS_EX_MC_BUTTON_NOTE:
        return _buttonNoteChannel;
        break;

        case SYS_EX_MC_LONG_PRESS_BUTTON_NOTE:
        return _longPressButtonNoteChannel;
        break;

        case SYS_EX_MC_POT_CC:
        return _potCCchannel;
        break;

        case SYS_EX_MC_POT_NOTE:
        return _potNoteChannel;
        break;

        case SYS_EX_MC_ENC_CC:
        return _encCCchannel;
        break;

        case SYS_EX_MC_INPUT:
        return _inputChannel;
        break;

        default:
        return 0;
        break;

    }

}

uint8_t OpenDeck::sysExGetHardwareParameter(uint8_t parameter)  {

    switch (parameter)  {

        case SYS_EX_HW_P_BOARD_TYPE:
        return _board;
        break;

        case SYS_EX_HW_P_LONG_PRESS_TIME:
        //long press time
        return _longPressTime/100;
        break;

        case SYS_EX_HW_P_BLINK_TIME:
        //blink time
        return _blinkTime/100;
        break;

        case SYS_EX_HW_P_TOTAL_LED_NUMBER:
        return totalNumberOfLEDs;
        break;

        case SYS_EX_HW_P_START_UP_SWITCH_TIME:
        //start-up led switch time
        startUpRoutine();
        return _startUpLEDswitchTime/10;
        break;

        case SYS_EX_HW_P_START_UP_ROUTINE:
        startUpRoutine();
        return startUpRoutinePattern;
        break;

        default:
        break;

    }   return 0;

}

bool OpenDeck::sysExGetFeature(uint8_t featureType, uint8_t feature)    {

    switch (featureType)    {

        case SYS_EX_MT_SW_FEATURE:
        return bitRead(softwareFeatures, feature);
        break;

        case SYS_EX_MT_HW_FEATURE:
        return bitRead(hardwareFeatures, feature);
        break;

        default:
        return false;
        break;

    }

    return false;

}

bool OpenDeck::sysExSet(uint8_t messageType, uint8_t messageSubType, uint8_t parameter, uint8_t newParameter)    {

    switch (messageType)    {

        case SYS_EX_MT_MIDI_CHANNEL:
        return sysExSetMIDIchannel(parameter, newParameter);
        break;

        case SYS_EX_MT_HW_PARAMETER:
        return sysExSetHardwareParameter(parameter, newParameter);
        break;
        
        case SYS_EX_MT_FREE_PINS:
        return configureFreePin(parameter, newParameter);
        break;

        case SYS_EX_MT_SW_FEATURE:
        case SYS_EX_MT_HW_FEATURE:
        return sysExSetFeature(messageType, parameter, newParameter);
        break;

        case SYS_EX_MT_BUTTON:
        if (messageSubType == SYS_EX_MST_BUTTON_TYPE)                   return sysExSetButtonType(parameter, newParameter);
        else                                                            return sysExSetButtonNote(parameter, newParameter);
        break;

        case SYS_EX_MT_POT:

        switch (messageSubType) {

            case SYS_EX_MST_POT_ENABLED:
            return sysExSetPotEnabled(parameter, newParameter);
            break;

            case SYS_EX_MST_POT_INVERTED:
            return sysExSetPotInvertState(parameter, newParameter);
            break;
            
            case SYS_EX_MST_POT_CC_NUMBER:
            return sysExSetCCnumber(parameter, newParameter);
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
        if (messageSubType == SYS_EX_MST_LED_ACT_NOTE)                  return sysExSetLEDnote(parameter, newParameter);
        else if (messageSubType == SYS_EX_MST_LED_START_UP_NUMBER)      return sysExSetLEDstartNumber(parameter, newParameter);
        else if (messageSubType == SYS_EX_MST_LED_STATE)    {

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

        }   else return false;

        break;

        default:
        return false;
        break;

    }
    
}

bool OpenDeck::sysExRestore(uint8_t messageType, uint8_t messageSubType, uint16_t parameter, int16_t componentNr) {

    uint16_t    eepromAddress = 0,
                _parameter    = 0;

    switch (messageType)    {

        case SYS_EX_MT_MIDI_CHANNEL:
        eepromAddress = EEPROM_MIDI_CHANNEL_START;
        break;

        case SYS_EX_MT_HW_PARAMETER:
        eepromAddress = EEPROM_HW_P_START;
        break;

        case SYS_EX_MT_SW_FEATURE:
        eepromAddress = EEPROM_SOFTWARE_FEATURES_START;
        break;

        case SYS_EX_MT_HW_FEATURE:
        eepromAddress = EEPROM_HARDWARE_FEATURES_START;
        break;

        case SYS_EX_MT_FREE_PINS:
        eepromAddress = EEPROM_FREE_PIN_START;
        break;

        case SYS_EX_MT_BUTTON:
        switch (messageSubType) {

            case SYS_EX_MST_BUTTON_TYPE:
            eepromAddress = EEPROM_BUTTON_TYPE_START;
            break;

            case SYS_EX_MST_BUTTON_NOTE:
            eepromAddress = EEPROM_BUTTON_NOTE_START;
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

            case SYS_EX_MST_POT_INVERTED:
            eepromAddress = EEPROM_POT_INVERSION_START;
            break;

            case SYS_EX_MST_POT_CC_NUMBER:
            eepromAddress = EEPROM_POT_CC_NUMBER_START;
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

        case SYS_EX_MT_ENC:
        return false;
        break;

        case SYS_EX_MT_LED:
        switch (messageSubType) {

            case SYS_EX_MST_LED_ACT_NOTE:
            eepromAddress = EEPROM_LED_ACT_NOTE_START;
            break;

            case SYS_EX_MST_LED_START_UP_NUMBER:
            eepromAddress = EEPROM_LED_START_UP_NUMBER_START;
            break;

            case SYS_EX_MST_LED_STATE:
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

        case SYS_EX_MC_ENC_CC:
        _encCCchannel = channelNumber;
        eeprom_update_byte((uint8_t*)EEPROM_MC_ENC_CC, channelNumber);
        return (channelNumber == eeprom_read_byte((uint8_t*)EEPROM_MC_ENC_CC));
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

bool OpenDeck::sysExSetHardwareParameter(uint8_t parameter, uint8_t value)  {

    switch (parameter)  {

        case SYS_EX_HW_P_BOARD_TYPE:
        _board = value;
        initBoard();
        eeprom_update_byte((uint8_t*)EEPROM_HW_P_BOARD_TYPE, value);
        return (eeprom_read_byte((uint8_t*)EEPROM_HW_P_BOARD_TYPE) == value);
        break;
        
        case SYS_EX_HW_P_LONG_PRESS_TIME:
        //long press time
        _longPressTime = value*100;
        eeprom_update_byte((uint8_t*)EEPROM_HW_P_LONG_PRESS_TIME, value);
        return (eeprom_read_byte((uint8_t*)EEPROM_HW_P_LONG_PRESS_TIME) == value);
        return true;
        break;

        case SYS_EX_HW_P_BLINK_TIME:
        //blink time
        _blinkTime = value*100;
        eeprom_update_byte((uint8_t*)EEPROM_HW_P_BLINK_TIME, value);
        return (eeprom_read_byte((uint8_t*)EEPROM_HW_P_BLINK_TIME) == value);
        break;

        case SYS_EX_HW_P_START_UP_SWITCH_TIME:
        //start-up led switch time
        _startUpLEDswitchTime = value*10;
        eeprom_update_byte((uint8_t*)EEPROM_HW_P_START_UP_SWITCH_TIME, value);
        startUpRoutine();
        return (eeprom_read_byte((uint8_t*)EEPROM_HW_P_START_UP_SWITCH_TIME) == value);
        break;

        case SYS_EX_HW_P_START_UP_ROUTINE:
        //set start-up routine pattern
        startUpRoutinePattern = value;
        eeprom_update_byte((uint8_t*)EEPROM_HW_P_START_UP_ROUTINE, value);
        startUpRoutine();
        return (eeprom_read_byte((uint8_t*)EEPROM_HW_P_START_UP_ROUTINE) == value);
        break;
        
        case SYS_EX_HW_P_TOTAL_LED_NUMBER:
        //set total number of LEDs (needed for start-up routine)
        totalNumberOfLEDs = value;
        eeprom_update_byte((uint8_t*)EEPROM_HW_P_TOTAL_LED_NUMBER, value);
        return (eeprom_read_byte((uint8_t*)EEPROM_HW_P_TOTAL_LED_NUMBER) == value);
        break;

        default:
        break;

    }   return false;

}

bool OpenDeck::sysExSetFreePin(uint8_t pin, uint8_t pinState)   {

    freePinState[pin] = pinState;
    eeprom_update_byte((uint8_t*)EEPROM_FREE_PIN_START+pin, pinState);
    return (eeprom_read_byte((uint8_t*)EEPROM_FREE_PIN_START+pin) == pinState);

}

bool OpenDeck::sysExSetFeature(uint8_t featureType, uint8_t feature, bool state)    {

    switch (featureType)    {

        case SYS_EX_MT_SW_FEATURE:
        //software feature
        bitWrite(softwareFeatures, feature, state);
        if ((feature == SYS_EX_SW_F_LED_BLINK) && (state == SYS_EX_DISABLE))
            for (int i=0; i<MAX_NUMBER_OF_LEDS; i++) handleLED(false, true, i); //remove all blinking bits from ledState
        eeprom_update_byte((uint8_t*)EEPROM_SOFTWARE_FEATURES_START, softwareFeatures);
        return (eeprom_read_byte((uint8_t*)EEPROM_SOFTWARE_FEATURES_START) == softwareFeatures);
        break;

        case SYS_EX_MT_HW_FEATURE:
        //hardware feature
        bitWrite(hardwareFeatures, feature, state);
        eeprom_update_byte((uint8_t*)EEPROM_HARDWARE_FEATURES_START, hardwareFeatures);
        return (eeprom_read_byte((uint8_t*)EEPROM_HARDWARE_FEATURES_START) == hardwareFeatures);
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

    return (buttonType[arrayIndex] == eeprom_read_byte((uint8_t*)eepromAddress));

}

bool OpenDeck::sysExSetButtonNote(uint8_t buttonNumber, uint8_t _buttonNote)    {

    uint16_t eepromAddress = EEPROM_BUTTON_NOTE_START+buttonNumber;

    buttonNote[buttonNumber] = _buttonNote;
    eeprom_update_byte((uint8_t*)eepromAddress, _buttonNote);
    return (_buttonNote == eeprom_read_byte((uint8_t*)eepromAddress));

}

bool OpenDeck::sysExSetPotEnabled(uint8_t potNumber, bool state)    {

    uint8_t arrayIndex = potNumber/8;
    uint8_t potIndex = potNumber - 8*arrayIndex;
    uint16_t eepromAddress = EEPROM_POT_ENABLED_START+arrayIndex;

    bitWrite(potEnabled[arrayIndex], potIndex, state);
    eeprom_update_byte((uint8_t*)eepromAddress, potEnabled[arrayIndex]);

    return (potEnabled[arrayIndex] == eeprom_read_byte((uint8_t*)eepromAddress));

}

bool OpenDeck::sysExSetPotInvertState(uint8_t potNumber, bool state)    {

    uint8_t arrayIndex = potNumber/8;
    uint8_t potIndex = potNumber - 8*arrayIndex;
    uint16_t eepromAddress = EEPROM_POT_INVERSION_START+arrayIndex;

    bitWrite(potInverted[arrayIndex], potIndex, state);
    eeprom_update_byte((uint8_t*)eepromAddress, potInverted[arrayIndex]);

    return (potInverted[arrayIndex] == eeprom_read_byte((uint8_t*)eepromAddress));

}

bool OpenDeck::sysExSetCCnumber(uint8_t potNumber, uint8_t _ccNumber)   {

    uint16_t eepromAddress = EEPROM_POT_CC_NUMBER_START+potNumber;

    ccNumber[potNumber] = _ccNumber;
    eeprom_update_byte((uint8_t*)eepromAddress, _ccNumber);
    return (_ccNumber == eeprom_read_byte((uint8_t*)eepromAddress));

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

bool OpenDeck::sysExSetLEDnote(uint8_t ledNumber, uint8_t _ledNote) {

    uint16_t eepromAddress = EEPROM_LED_ACT_NOTE_START+ledNumber;

    ledNote[ledNumber] = _ledNote;
    eeprom_update_byte((uint8_t*)eepromAddress, _ledNote);
    return _ledNote == eeprom_read_byte((uint8_t*)eepromAddress);

}

bool OpenDeck::sysExSetLEDstartNumber(uint8_t ledNumber, uint8_t startNumber) {

    uint16_t eepromAddress = EEPROM_LED_START_UP_NUMBER_START+ledNumber;

    eeprom_update_byte((uint8_t*)eepromAddress, startNumber);
    return startNumber == eeprom_read_byte((uint8_t*)eepromAddress);

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