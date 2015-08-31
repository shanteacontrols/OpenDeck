/*

OpenDECK library v1.3
File: SysEx.cpp
Last revision date: 2014-12-25
Author: Igor Petrovic

*/

#include "OpenDeck.h"
#include <avr/eeprom.h>
#include "LEDsettings.h"
#include "EncoderSettings.h"
#include "SysEx.h"

bool OpenDeck::sysExCheckMessageValidity(uint8_t sysExArray[], uint8_t arrSize)  {

    //don't respond to sysex message if device ID is wrong
    if (!sysExCheckID(sysExArray[SYS_EX_MS_M_ID_0], sysExArray[SYS_EX_MS_M_ID_1], sysExArray[SYS_EX_MS_M_ID_2])) return false;

    //only check rest of the message if it's not just a ID check and controller has received handshake
    if (!((arrSize >= SYS_EX_ML_REQ_STANDARD) && (sysExEnabled))) {

        if (arrSize == (SYS_EX_ML_REQ_HANDSHAKE+1))   {

            if (sysExArray[arrSize-2] == 0x7F)  {

                #ifdef USBCON
                    //this only works on USB boards
                    //turn off all leds slowly before reseting
                    boardObject.setLEDTransitionSpeed(1);
                    allLEDsOff();
                    //make sure all leds are off
                    boardObject.newDelay(1000);
                    //reboot
                    boardObject.rebootBoard();
                #endif

            }

        }

        if (arrSize != SYS_EX_ML_REQ_HANDSHAKE) {

            if (sysExEnabled)   sysExGenerateError(SYS_EX_ERROR_MESSAGE_LENGTH);
            else                sysExGenerateError(SYS_EX_ERROR_HANDSHAKE);
            return false;

        }   return true;

    }

    //check wish validity
    if (!sysExCheckWish(sysExArray[SYS_EX_MS_WISH]))    {

        sysExGenerateError(SYS_EX_ERROR_WISH);
        return false;

    }

    //check if wanted amount is correct
    if (!sysExCheckAmount(sysExArray[SYS_EX_MS_AMOUNT]))    {

        sysExGenerateError(SYS_EX_ERROR_AMOUNT);
        return false;

    }

    //check if message type is correct
    if (!sysExCheckMessageType(sysExArray[SYS_EX_MS_MT]))    {

        sysExGenerateError(SYS_EX_ERROR_MT);
        return false;

    } else {

        //check for special wish/amount/message type combinations
        if (!(sysExCheckSpecial(sysExArray[SYS_EX_MS_MT], sysExArray[SYS_EX_MS_WISH], sysExArray[SYS_EX_MS_AMOUNT])))
            return false;

        //determine minimum message length based on asked parameters
        if (arrSize < sysExGenerateMinMessageLenght(sysExArray[SYS_EX_MS_WISH], sysExArray[SYS_EX_MS_AMOUNT],sysExArray[SYS_EX_MS_MT], sysExArray[SYS_EX_MS_MST]))    {

            sysExGenerateError(SYS_EX_ERROR_MESSAGE_LENGTH);
            return false;

        }

    }

    //check if subtype is correct
    if (!sysExCheckMessageSubType(sysExArray[SYS_EX_MS_MT], sysExArray[SYS_EX_MS_MST])) {

        sysExGenerateError(SYS_EX_ERROR_MST);
        return false;

    }

    //check if wanted parameter is valid only if single parameter is specified
    if (sysExArray[SYS_EX_MS_AMOUNT] == SYS_EX_AMOUNT_SINGLE)   {

        if (!sysExCheckParameterID(sysExArray[SYS_EX_MS_MT], sysExArray[SYS_EX_MS_MST], sysExArray[SYS_EX_MS_PARAMETER_ID]))  {

            sysExGenerateError(SYS_EX_ERROR_PARAMETER);
            return false;

        }

        //if message wish is set, check new parameter
        if (sysExArray[SYS_EX_MS_WISH] == SYS_EX_WISH_SET) {

            if (!sysExCheckNewParameterID(sysExArray[SYS_EX_MS_MT], sysExArray[SYS_EX_MS_MST], sysExArray[SYS_EX_MS_PARAMETER_ID], sysExArray[SYS_EX_MS_NEW_PARAMETER_ID_SINGLE]))  {

                sysExGenerateError(SYS_EX_ERROR_NEW_PARAMETER);
                return false;

            }

        }

    } else {

        //all parameters

        //check each new parameter for set command
        if (sysExArray[SYS_EX_MS_WISH] == SYS_EX_WISH_SET) {

            uint8_t arrayIndex = SYS_EX_MS_NEW_PARAMETER_ID_ALL;

            for (int i=0; i<(arrSize - arrayIndex)-1; i++)

            if (!sysExCheckNewParameterID(sysExArray[SYS_EX_MS_MT], sysExArray[SYS_EX_MS_MST], i, sysExArray[arrayIndex+i]))   {

                sysExGenerateError(SYS_EX_ERROR_NEW_PARAMETER);
                return false;

            }

        }

    }

    return true;

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

        case SYS_EX_MT_FEATURES:
        return ((messageSubType >= SYS_EX_MST_FEATURES_START) && (messageSubType < SYS_EX_MST_FEATURES_END));
        break;

        case SYS_EX_MT_BUTTONS:
        return ((messageSubType >= SYS_EX_MST_BUTTONS_START) && (messageSubType < SYS_EX_MST_BUTTONS_END));
        break;

        case SYS_EX_MT_ANALOG:
        return ((messageSubType >= SYS_EX_MST_ANALOG_START) && (messageSubType < SYS_EX_MST_ANALOG_END));
        break;

        case SYS_EX_MT_LEDS:
        return ((messageSubType >= SYS_EX_MST_LEDS_START) && (messageSubType < SYS_EX_MST_LEDS_END));
        break;

        case SYS_EX_MT_ENCODERS:
        return ((messageSubType >= SYS_EX_MST_ENCODERS_START) && (messageSubType < SYS_EX_MST_ENCODERS_END));

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

        case SYS_EX_MT_FEATURES:
        switch (messageSubType) {

            case SYS_EX_MST_FEATURES_MIDI:
            return ((parameter >= SYS_EX_FEATURES_MIDI_START) && (parameter < SYS_EX_FEATURES_MIDI_END));
            break;

            case SYS_EX_MST_FEATURES_LEDS:
            return ((parameter >= SYS_EX_FEATURES_LEDS_START) && (parameter < SYS_EX_FEATURES_LEDS_END));
            break;

            case SYS_EX_MST_FEATURES_ANALOG:
            return ((parameter >= SYS_EX_FEATURES_ANALOG_START) && (parameter < SYS_EX_FEATURES_ANALOG_END));
            break;

            case SYS_EX_MST_FEATURES_ENCODERS:
            return ((parameter >= SYS_EX_FEATURES_ENCODERS_START) && (parameter < SYS_EX_FEATURES_ENCODERS_END));
            break;

            default:
            return false;
            break;

        }

        case SYS_EX_MT_MIDI_CHANNEL:
        return ((parameter >= SYS_EX_MC_START) && (parameter < SYS_EX_MC_END));
        break;

        case SYS_EX_MT_BUTTONS:
        switch (messageSubType) {

            case SYS_EX_MST_BUTTONS_TYPE:
            case SYS_EX_MST_BUTTONS_PROGRAM_CHANGE_ENABLED:
            case SYS_EX_MST_BUTTONS_NOTE:
            return (parameter < MAX_NUMBER_OF_BUTTONS);
            break;

            default:
            return false;
            break;

        }

        case SYS_EX_MT_ANALOG:
        return (parameter < MAX_NUMBER_OF_ANALOG);
        break;

        case SYS_EX_MT_ENCODERS:
        return (parameter < NUMBER_OF_ENCODERS);
        break;

        case SYS_EX_MT_LEDS:
        switch (messageSubType) {

            case SYS_EX_MST_LEDS_ACT_NOTE:
            case SYS_EX_MST_LEDS_STATE:
            return  (parameter < MAX_NUMBER_OF_LEDS);
            break;

            case SYS_EX_MST_LEDS_START_UP_NUMBER:
            return (parameter < totalNumberOfLEDs);
            break;

            case SYS_EX_MST_LEDS_HW_P:
            return ((parameter >= SYS_EX_LEDS_HW_P_START) && (parameter < SYS_EX_LEDS_HW_P_END));
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

        case SYS_EX_MT_FEATURES:
        return ((newParameter == SYS_EX_ENABLE) || (newParameter == SYS_EX_DISABLE));
        break;

        case SYS_EX_MT_MIDI_CHANNEL:
        return ((newParameter >= 1) && (newParameter <= 16));   //there are only 16 MIDI channels
        break;

        case SYS_EX_MT_BUTTONS:
        switch (messageSubType) {

            case SYS_EX_MST_BUTTONS_TYPE:
            case SYS_EX_MST_BUTTONS_PROGRAM_CHANGE_ENABLED:
            return ((newParameter == SYS_EX_ENABLE) || (newParameter == SYS_EX_DISABLE));
            break;

            case SYS_EX_MST_BUTTONS_NOTE:
            return (newParameter < 128);
            break;

            break;

            default:
            return false;
            break;

        }

        case SYS_EX_MT_ANALOG:
        switch (messageSubType) {

            case SYS_EX_MST_ANALOG_ENABLED:
            case SYS_EX_MST_ANALOG_INVERTED:
            return ((newParameter == SYS_EX_ENABLE) || (newParameter == SYS_EX_DISABLE));
            break;

            case SYS_EX_MST_ANALOG_TYPE:
            return ((newParameter >= SYS_EX_ANALOG_TYPE_START) && (newParameter < SYS_EX_ANALOG_TYPE_END));
            break;

            case SYS_EX_MST_ANALOG_NUMBER:
            case SYS_EX_MST_ANALOG_LOWER_LIMIT:
            case SYS_EX_MST_ANALOG_UPPER_LIMIT:
            return (newParameter < 128);
            break;

            default:
            return false;
            break;

        }

        case SYS_EX_MT_ENCODERS:
        switch(messageSubType)  {

            case SYS_EX_MST_ENCODERS_ENABLED:
            case SYS_EX_MST_ENCODERS_INVERTED:
            case SYS_EX_MST_ENCODERS_FAST_MODE:
            return ((newParameter == SYS_EX_ENABLE) || (newParameter == SYS_EX_DISABLE));
            break;

            case SYS_EX_MST_ENCODERS_NUMBER:
            return (newParameter < 128);
            break;

            case SYS_EX_MST_ENCODERS_PULSES_PER_STEP:
            return ((newParameter >= PULSES_PER_STEP_MIN) && (newParameter <= PULSES_PER_STEP_MAX));
            break;

        }

        break;

        case SYS_EX_MT_LEDS:
        switch (messageSubType)  {

            case SYS_EX_MST_LEDS_ACT_NOTE:
            case SYS_EX_MST_LEDS_START_UP_NUMBER:
            return ((checkSameLEDvalue(messageSubType, newParameter)) && (newParameter < 128));
            break;

            case SYS_EX_MST_LEDS_STATE:
            return ((newParameter >= SYS_EX_LEDS_STATE_START)  && (newParameter < SYS_EX_LEDS_STATE_END));
            break;

            case SYS_EX_MST_LEDS_HW_P:
            switch (parameter)  {

                case SYS_EX_LEDS_HW_P_TOTAL_NUMBER:
                return (newParameter < MAX_NUMBER_OF_LEDS);
                break;

                case SYS_EX_LEDS_HW_P_BLINK_TIME:
                return ((newParameter >= BLINK_TIME_MIN) && (newParameter <= BLINK_TIME_MAX));
                break;

                case SYS_EX_LEDS_HW_P_START_UP_SWITCH_TIME:
                return ((newParameter >= START_UP_SWITCH_TIME_MIN) && (newParameter <= START_UP_SWITCH_TIME_MAX));
                break;

                case SYS_EX_LEDS_HW_P_START_UP_ROUTINE:
                return (newParameter < NUMBER_OF_START_UP_ROUTINES);
                break;

                case SYS_EX_LEDS_HW_P_FADE_TIME:
                return ((newParameter >= FADE_TIME_MIN) && (newParameter <= FADE_TIME_MAX));
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

                case SYS_EX_MT_FEATURES:
                switch (messageSubType) {

                    case SYS_EX_MST_FEATURES_MIDI:
                    return SYS_EX_ML_REQ_STANDARD + SYS_EX_FEATURES_MIDI_END;
                    break;

                    case SYS_EX_MST_FEATURES_LEDS:
                    return SYS_EX_ML_REQ_STANDARD + SYS_EX_FEATURES_LEDS_END;
                    break;

                    case SYS_EX_MST_FEATURES_ANALOG:
                    return SYS_EX_ML_REQ_STANDARD + SYS_EX_FEATURES_ANALOG_END;
                    break;

                    case SYS_EX_MST_FEATURES_ENCODERS:
                    return SYS_EX_ML_REQ_STANDARD + SYS_EX_FEATURES_ENCODERS_END;
                    break;

                    default:
                    return 0;
                    break;

                }

                break;

                case SYS_EX_MT_MIDI_CHANNEL:
                return SYS_EX_ML_REQ_STANDARD + SYS_EX_MC_END;
                break;

                case SYS_EX_MT_BUTTONS:
                switch (messageSubType) {

                    case SYS_EX_MST_BUTTONS_TYPE:
                    case SYS_EX_MST_BUTTONS_PROGRAM_CHANGE_ENABLED:
                    case SYS_EX_MST_BUTTONS_NOTE:
                    return SYS_EX_ML_REQ_STANDARD + MAX_NUMBER_OF_BUTTONS;
                    break;

                    default:
                    return 0;
                    break;

                }

                break;

                case SYS_EX_MT_ANALOG:
                switch (messageSubType) {

                    case SYS_EX_MST_ANALOG_HW_P:
                    return SYS_EX_ML_REQ_STANDARD + SYS_EX_ANALOG_HW_P_END;
                    break;

                    case SYS_EX_MST_ANALOG_ENABLED:
                    case SYS_EX_MST_ANALOG_INVERTED:
                    case SYS_EX_MST_ANALOG_NUMBER:
                    case SYS_EX_MST_ANALOG_LOWER_LIMIT:
                    case SYS_EX_MST_ANALOG_UPPER_LIMIT:
                    case SYS_EX_MST_ANALOG_TYPE:
                    return SYS_EX_ML_REQ_STANDARD + MAX_NUMBER_OF_ANALOG;
                    break;

                    default:
                    return 0;
                    break;

                }

                break;

                case SYS_EX_MT_ENCODERS:
                switch (messageSubType) {

                    case SYS_EX_MST_ENCODERS_HW_P:
                    return SYS_EX_ML_REQ_STANDARD + SYS_EX_ENCODERS_HW_P_END;
                    break;

                    case SYS_EX_MST_ENCODERS_ENABLED:
                    case SYS_EX_MST_ENCODERS_INVERTED:
                    case SYS_EX_MST_ENCODERS_FAST_MODE:
                    case SYS_EX_MST_ENCODERS_PULSES_PER_STEP:
                    case SYS_EX_MST_ENCODERS_NUMBER:
                    return SYS_EX_ML_REQ_STANDARD + NUMBER_OF_ENCODERS;
                    break;

                    default:
                    return 0;
                    break;

                }

                break;

                case SYS_EX_MT_LEDS:
                switch (messageSubType) {

                    case SYS_EX_MST_LEDS_ACT_NOTE:
                    case SYS_EX_MST_LEDS_STATE:
                    return SYS_EX_ML_REQ_STANDARD + MAX_NUMBER_OF_LEDS;
                    break;

                    case SYS_EX_MST_LEDS_START_UP_NUMBER:
                    return totalNumberOfLEDs;
                    break;

                    case SYS_EX_MST_LEDS_HW_P:
                    return SYS_EX_LEDS_HW_P_END;
                    break;

                    default:
                    return 0;
                    break;

                }

                break;

                default:
                return 0;
                break;

            }

        }

    }   else return 0;

}

void OpenDeck::sysExGenerateError(sysExError errorID)  {

    uint8_t sysExResponse[5];

    sysExResponse[0] = SYS_EX_M_ID_0;
    sysExResponse[1] = SYS_EX_M_ID_1;
    sysExResponse[2] = SYS_EX_M_ID_2;
    sysExResponse[3] = SYS_EX_NACK;
    sysExResponse[4] = errorID;

    sendSysEx(sysExResponse, 5);

}

void OpenDeck::sysExGenerateAck()   {

    uint8_t sysExAckResponse[7];

    sysExAckResponse[0] = SYS_EX_M_ID_0;
    sysExAckResponse[1] = SYS_EX_M_ID_1;
    sysExAckResponse[2] = SYS_EX_M_ID_2;
    sysExAckResponse[3] = VERSION_BYTE_0;
    sysExAckResponse[4] = VERSION_BYTE_1;
    sysExAckResponse[5] = VERSION_BYTE_2;
    sysExAckResponse[6] = SYS_EX_ACK;

    sysExEnabled = true;

    sendSysEx(sysExAckResponse, 7);

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
    uint8_t sysExResponse[64+SYS_EX_ML_RES_BASIC];

    //copy first part of request to response
    for (int i=0; i<(SYS_EX_ML_RES_BASIC-1); i++)
        sysExResponse[i] = sysExArray[i+1];

    sysExResponse[SYS_EX_ML_RES_BASIC-1] = SYS_EX_ACK;

    if (sysExArray[SYS_EX_MS_AMOUNT] == SYS_EX_AMOUNT_ALL) {

        switch (sysExArray[SYS_EX_MS_MT])   {

            case SYS_EX_MT_FEATURES:
            switch (sysExArray[SYS_EX_MS_MST])  {

                case SYS_EX_MST_FEATURES_MIDI:
                maxComponentNr = SYS_EX_FEATURES_MIDI_END;
                break;

                case SYS_EX_MST_FEATURES_LEDS:
                maxComponentNr = SYS_EX_FEATURES_LEDS_END;
                break;

                case SYS_EX_MST_FEATURES_ANALOG:
                maxComponentNr = SYS_EX_FEATURES_ANALOG_END;
                break;

                case SYS_EX_MST_FEATURES_ENCODERS:
                maxComponentNr = SYS_EX_FEATURES_ENCODERS_END;
                break;

            }

            break;

            case SYS_EX_MT_MIDI_CHANNEL:
            maxComponentNr = SYS_EX_MC_END;
            break;

            case SYS_EX_MT_BUTTONS:
            switch (sysExArray[SYS_EX_MS_MST])  {

                case SYS_EX_MST_BUTTONS_TYPE:
                case SYS_EX_MST_BUTTONS_PROGRAM_CHANGE_ENABLED:
                case SYS_EX_MST_BUTTONS_NOTE:
                maxComponentNr = MAX_NUMBER_OF_BUTTONS;
                break;

                default:
                break;

            }

            break;

            case SYS_EX_MT_ANALOG:
            switch (sysExArray[SYS_EX_MS_MST])  {

                case SYS_EX_MST_ANALOG_ENABLED:
                case SYS_EX_MST_ANALOG_INVERTED:
                case SYS_EX_MST_ANALOG_NUMBER:
                case SYS_EX_MST_ANALOG_LOWER_LIMIT:
                case SYS_EX_MST_ANALOG_UPPER_LIMIT:
                maxComponentNr = MAX_NUMBER_OF_ANALOG;
                break;

                case SYS_EX_MST_ANALOG_HW_P:
                maxComponentNr = SYS_EX_ANALOG_HW_P_END;
                break;

                default:
                break;

            }

            break;

            case SYS_EX_MT_LEDS:
            switch (sysExArray[SYS_EX_MS_MST])  {

                case SYS_EX_MST_LEDS_ACT_NOTE:
                case SYS_EX_MST_LEDS_START_UP_NUMBER:
                case SYS_EX_MST_LEDS_STATE:
                maxComponentNr = MAX_NUMBER_OF_LEDS;
                break;

                case SYS_EX_MST_LEDS_HW_P:
                maxComponentNr = SYS_EX_LEDS_HW_P_END;
                break;

                default:
                break;

            }

            break;

            case SYS_EX_MT_ALL:
            maxComponentNr = (int16_t)sizeof(defConf);
            break;

            case SYS_EX_MT_ENCODERS:
            switch (sysExArray[SYS_EX_MS_MST])  {

                case SYS_EX_MST_ENCODERS_ENABLED:
                case SYS_EX_MST_ENCODERS_FAST_MODE:
                case SYS_EX_MST_ENCODERS_INVERTED:
                case SYS_EX_MST_ENCODERS_NUMBER:
                case SYS_EX_MST_ENCODERS_PULSES_PER_STEP:
                maxComponentNr = NUMBER_OF_ENCODERS;
                break;

                case SYS_EX_MST_ENCODERS_HW_P:
                maxComponentNr = SYS_EX_ENCODERS_HW_P_END;
                break;

                default:
                break;

            }

            default:
            break;

        }   componentNr = maxComponentNr;

    }

    //create response based on wanted message type
    if (sysExArray[SYS_EX_MS_WISH] == SYS_EX_WISH_GET)    {                     //get

        if (sysExArray[SYS_EX_MS_AMOUNT] == SYS_EX_AMOUNT_ALL)  _parameter = 0;
        else                                                    _parameter = sysExArray[SYS_EX_MS_PARAMETER_ID];

        for (int i=0; i<componentNr; i++) {

            sysExResponse[i+SYS_EX_ML_RES_BASIC] = sysExGet(sysExArray[SYS_EX_MS_MT], sysExArray[SYS_EX_MS_MST], _parameter);
            _parameter++;

        }

        sendSysEx(sysExResponse, SYS_EX_ML_RES_BASIC+componentNr);
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

            sendSysEx(sysExResponse, SYS_EX_ML_RES_BASIC);
            return;

        }   else if (sysExArray[SYS_EX_MS_WISH] == SYS_EX_WISH_RESTORE) {       //restore

                if (!sysExRestore(sysExArray[SYS_EX_MS_MT], sysExArray[SYS_EX_MS_MST], sysExArray[SYS_EX_MS_PARAMETER_ID], componentNr)) {

                    sysExGenerateError(SYS_EX_ERROR_EEPROM);
                    return;

                }

                sendSysEx(sysExResponse, SYS_EX_ML_RES_BASIC);
                return;

            }
}

uint8_t OpenDeck::sysExGet(uint8_t messageType, uint8_t messageSubType, uint8_t parameter)  {

    switch (messageType)    {

        case SYS_EX_MT_FEATURES:
        return getFeature(messageSubType, parameter);
        break;

        case SYS_EX_MT_MIDI_CHANNEL:
        return getMIDIchannel(parameter);
        break;

        case SYS_EX_MT_BUTTONS:
        switch (messageSubType) {

            case SYS_EX_MST_BUTTONS_TYPE:
            return getButtonType(parameter);
            break;

            case SYS_EX_MST_BUTTONS_PROGRAM_CHANGE_ENABLED:
            return getButtonPCenabled(parameter);
            break;

            case SYS_EX_MST_BUTTONS_NOTE:
            return noteNumber[parameter];
            break;

            default:
            return false;
            break;

        }

        break;

        case SYS_EX_MT_ANALOG:

        switch (messageSubType) {

            case SYS_EX_MST_ANALOG_ENABLED:
            return getAnalogEnabled(parameter);
            break;

            case SYS_EX_MST_ANALOG_TYPE:
            return getAnalogType(parameter);
            break;

            case SYS_EX_MST_ANALOG_INVERTED:
            return getAnalogInvertState(parameter);
            break;

            case SYS_EX_MST_ANALOG_NUMBER:
            return ccNumber[parameter];
            break;

            case SYS_EX_MST_ANALOG_LOWER_LIMIT:
            return analogLowerLimit[parameter];
            break;

            case SYS_EX_MST_ANALOG_UPPER_LIMIT:
            return analogUpperLimit[parameter];
            break;

            default:
            return false;
            break;

        }

        break;

        case SYS_EX_MT_LEDS:
        switch (messageSubType) {

            case SYS_EX_MST_LEDS_ACT_NOTE:
            return ledActNote[parameter];
            break;

            case SYS_EX_MST_LEDS_START_UP_NUMBER:
            return eeprom_read_byte((uint8_t*)EEPROM_LEDS_START_UP_NUMBER_START+parameter);
            break;

            case SYS_EX_MST_LEDS_STATE:
            return boardObject.getLEDstate(parameter);
            break;

            case SYS_EX_MST_LEDS_HW_P:
            return getLEDHwParameter(parameter);
            break;

            default:
            return false;
            break;

        }

        break;

        case SYS_EX_MT_ENCODERS:
        switch (messageSubType) {

            case SYS_EX_MST_ENCODERS_ENABLED:
            return getEncoderEnabled(parameter);
            break;

            case SYS_EX_MST_ENCODERS_INVERTED:
            return getEncoderInvertState(parameter);
            break;

            case SYS_EX_MST_ENCODERS_NUMBER:
            return encoderNumber[parameter];
            break;

            case SYS_EX_MST_ENCODERS_PULSES_PER_STEP:
            return pulsesPerStep[parameter];
            break;

            case SYS_EX_MST_ENCODERS_FAST_MODE:
            return getEncoderFastMode(parameter);
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



bool OpenDeck::getFeature(uint8_t featureType, uint8_t feature)    {

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

        case SYS_EX_MST_FEATURES_ANALOG:
        return bitRead(analogFeatures, feature);
        break;

        case SYS_EX_MST_FEATURES_ENCODERS:
        return bitRead(encoderFeatures, feature);
        break;

        default:
        return false;
        break;

    }

    return false;

}

uint8_t OpenDeck::getMIDIchannel(uint8_t channel)  {

    switch (channel)    {

        case SYS_EX_MC_BUTTON_NOTE:
        return _noteChannel;
        break;

        case SYS_EX_MC_PROGRAM_CHANGE:
        return _programChangeChannel;
        break;

        case SYS_EX_MC_CC:
        return _CCchannel;
        break;

        case SYS_EX_MC_INPUT:
        return _inputChannel;
        break;

        default:
        return 0;
        break;

    }

}

uint8_t OpenDeck::getLEDHwParameter(uint8_t parameter)  {

    switch (parameter)  {

        case SYS_EX_LEDS_HW_P_TOTAL_NUMBER:
        return totalNumberOfLEDs;
        break;

        case SYS_EX_LEDS_HW_P_BLINK_TIME:
        return eeprom_read_byte((uint8_t*)EEPROM_LEDS_HW_P_BLINK_TIME);
        break;

        case SYS_EX_LEDS_HW_P_START_UP_SWITCH_TIME:
        return eeprom_read_byte((uint8_t*)EEPROM_LEDS_HW_P_START_UP_SWITCH_TIME);
        break;

        case SYS_EX_LEDS_HW_P_START_UP_ROUTINE:
        return eeprom_read_byte((uint8_t*)EEPROM_LEDS_HW_P_START_UP_ROUTINE);
        break;

        case SYS_EX_LEDS_HW_P_FADE_TIME:
        return eeprom_read_byte((uint8_t*)EEPROM_LEDS_HW_P_FADE_SPEED);
        break;

        default:
        return 0;
        break;

    }

}

bool OpenDeck::sysExSet(uint8_t messageType, uint8_t messageSubType, uint8_t parameter, uint8_t newParameter)    {

    switch (messageType)    {

        case SYS_EX_MT_MIDI_CHANNEL:
        return setMIDIchannel(parameter, newParameter);
        break;

        case SYS_EX_MT_FEATURES:
        return setFeature(messageType, parameter, newParameter);
        break;

        case SYS_EX_MT_BUTTONS:
        switch (messageSubType) {

            case SYS_EX_MST_BUTTONS_TYPE:
            return setButtonType(parameter, newParameter);
            break;

            case SYS_EX_MST_BUTTONS_PROGRAM_CHANGE_ENABLED:
            return setButtonPCenabled(parameter, newParameter);
            break;

            case SYS_EX_MST_BUTTONS_NOTE:
            return setButtonNote(parameter, newParameter);
            break;

            default:
            return false;
            break;

        }

        break;

        case SYS_EX_MT_ANALOG:
        switch (messageSubType) {

            case SYS_EX_MST_ANALOG_ENABLED:
            return setAnalogEnabled(parameter, newParameter);
            break;

            case SYS_EX_MST_ANALOG_TYPE:
            return setAnalogType(parameter, newParameter);
            break;

            case SYS_EX_MST_ANALOG_INVERTED:
            return setAnalogInvertState(parameter, newParameter);
            break;

            case SYS_EX_MST_ANALOG_NUMBER:
            return setCCnumber(parameter, newParameter);
            break;

            case SYS_EX_MST_ANALOG_LOWER_LIMIT:
            case SYS_EX_MST_ANALOG_UPPER_LIMIT:
            return setAnalogLimit(messageSubType, parameter, newParameter);
            break;

            default:
            return false;
            break;

        }

        break;

        case SYS_EX_MT_ENCODERS:
        switch(messageSubType)  {

            case SYS_EX_MST_ENCODERS_ENABLED:
            return setEncoderEnabled(parameter, newParameter);
            break;

            case SYS_EX_MST_ENCODERS_INVERTED:
            return setEncoderInvertState(parameter, newParameter);
            break;

            case SYS_EX_MST_ENCODERS_NUMBER:
            return setEncoderNumber(parameter, newParameter);
            break;

            case SYS_EX_MST_ENCODERS_PULSES_PER_STEP:
            return setEncoderPulsesPerStep(parameter, newParameter);
            break;

            case SYS_EX_MST_ENCODERS_FAST_MODE:
            return setEncoderFastMode(parameter, newParameter);
            break;

            default:
            return false;
            break;

        }

        break;

        case SYS_EX_MT_LEDS:
        switch(messageSubType)  {

            case SYS_EX_MST_LEDS_ACT_NOTE:
            return setLEDActivationNote(parameter, newParameter);
            break;

            case SYS_EX_MST_LEDS_START_UP_NUMBER:
            return setLEDstartNumber(parameter, newParameter);
            break;

            case SYS_EX_MST_LEDS_HW_P:
            return setLEDHwParameter(parameter, newParameter);
            break;

            case SYS_EX_MST_LEDS_STATE:
            switch (newParameter)   {

                case SYS_EX_LEDS_STATE_C_OFF:
                handleLED(false, false, parameter);
                return true;
                break;

                case SYS_EX_LEDS_STATE_C_ON:
                handleLED(true, false, parameter);
                return true;
                break;

                case SYS_EX_LEDS_STATE_B_OFF:
                handleLED(false, true, parameter);
                return true;
                break;

                case SYS_EX_LEDS_STATE_B_ON:
                handleLED(true, true, parameter);
                return true;
                break;

                default:
                return false;
                break;

            }

            break;

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

            case SYS_EX_MST_FEATURES_ANALOG:
            eepromAddress = EEPROM_FEATURES_ANALOG;
            break;
            
            case SYS_EX_MST_FEATURES_ENCODERS:
            eepromAddress = EEPROM_FEATURES_ENCODERS;
            break;

            default:
            return false;
            break;

        }

        break;

        case SYS_EX_MT_MIDI_CHANNEL:
        eepromAddress = EEPROM_MC_START;
        break;

        case SYS_EX_MT_BUTTONS:
        switch (messageSubType) {

            case SYS_EX_MST_BUTTONS_TYPE:
            eepromAddress = EEPROM_BUTTONS_TYPE_START;
            break;

            case SYS_EX_MST_BUTTONS_PROGRAM_CHANGE_ENABLED:
            eepromAddress = EEPROM_BUTTONS_PC_ENABLED_START;
            break;

            case SYS_EX_MST_BUTTONS_NOTE:
            eepromAddress = EEPROM_BUTTONS_NOTE_START;
            break;

            case SYS_EX_MST_BUTTONS_HW_P:
            eepromAddress = EEPROM_BUTTONS_HW_P_START;
            break;

            default:
            return false;
            break;

        }

        break;

        case SYS_EX_MT_ANALOG:
        switch (messageSubType) {

            case SYS_EX_MST_ANALOG_ENABLED:
            eepromAddress = EEPROM_ANALOG_ENABLED_START;
            break;

            case SYS_EX_MST_ANALOG_TYPE:
            eepromAddress = EEPROM_ANALOG_TYPE_START;
            break;

            case SYS_EX_MST_ANALOG_INVERTED:
            eepromAddress = EEPROM_ANALOG_INVERTED_START;
            break;

            case SYS_EX_MST_ANALOG_NUMBER:
            eepromAddress = EEPROM_ANALOG_NUMBER_START;
            break;

            case SYS_EX_MST_ANALOG_LOWER_LIMIT:
            eepromAddress = EEPROM_ANALOG_LOWER_LIMIT_START;
            break;

            case SYS_EX_MST_ANALOG_UPPER_LIMIT:
            eepromAddress = EEPROM_ANALOG_UPPER_LIMIT_START;
            break;

            default:
            return false;
            break;

        }

        break;

        case SYS_EX_MT_ENCODERS:
        switch(messageSubType)  {

            case SYS_EX_MST_ENCODERS_ENABLED:
            eepromAddress = EEPROM_ENCODERS_ENABLED_START;
            break;

            case SYS_EX_MST_ENCODERS_INVERTED:
            eepromAddress = EEPROM_ENCODERS_INVERTED_START;
            break;

            case SYS_EX_MST_ENCODERS_NUMBER:
            eepromAddress = EEPROM_ENCODERS_NUMBER_START;
            break;

            case SYS_EX_MST_ENCODERS_PULSES_PER_STEP:
            eepromAddress = EEPROM_ENCODERS_PULSES_PER_STEP_START;
            break;

            default:
            return false;
            break;

        }

        case SYS_EX_MT_LEDS:
        switch (messageSubType) {

            case SYS_EX_MST_LEDS_ACT_NOTE:
            eepromAddress = EEPROM_LEDS_ACT_NOTE_START;
            break;

            case SYS_EX_MST_LEDS_START_UP_NUMBER:
            eepromAddress = EEPROM_LEDS_START_UP_NUMBER_START;
            break;

            case SYS_EX_MST_LEDS_HW_P:
            eepromAddress = EEPROM_LEDS_HW_P_START;
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

    if (messageSubType == SYS_EX_MST_LEDS_STATE) {

        for (int i=0; i<MAX_NUMBER_OF_LEDS; i++)    boardObject.setLEDstate(i, 0);
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

bool OpenDeck::setMIDIchannel(uint8_t channel, uint8_t channelNumber)  {

    switch (channel)    {

        case SYS_EX_MC_BUTTON_NOTE:
        _noteChannel = channelNumber;
        eeprom_update_byte((uint8_t*)EEPROM_MC_NOTE, channelNumber);
        return (channelNumber == eeprom_read_byte((uint8_t*)EEPROM_MC_NOTE));
        break;

        case SYS_EX_MC_CC:
        _CCchannel = channelNumber;
        eeprom_update_byte((uint8_t*)EEPROM_MC_CC, channelNumber);
        return (channelNumber == eeprom_read_byte((uint8_t*)EEPROM_MC_CC));
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


bool OpenDeck::setFeature(uint8_t featureType, uint8_t feature, bool state)    {

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
        //if ((feature == SYS_EX_FEATURES_LEDS_BLINK) && (state == SYS_EX_DISABLE))
        //for (int i=0; i<MAX_NUMBER_OF_LEDS; i++) handleLED(false, true, i); //remove all blinking bits from ledState
        eeprom_update_byte((uint8_t*)EEPROM_FEATURES_LEDS, ledFeatures);
        return (eeprom_read_byte((uint8_t*)EEPROM_FEATURES_LEDS) == ledFeatures);
        break;

        case SYS_EX_MST_FEATURES_ANALOG:
        //analog features
        bitWrite(analogFeatures, feature, state);
        eeprom_update_byte((uint8_t*)EEPROM_FEATURES_ANALOG, analogFeatures);
        return (eeprom_read_byte((uint8_t*)EEPROM_FEATURES_ANALOG) == analogFeatures);
        break;

        default:
        break;

    }   return false;

}

bool OpenDeck::setButtonType(uint8_t buttonNumber, bool type)  {

    uint8_t arrayIndex = buttonNumber/8;
    uint8_t buttonIndex = buttonNumber - 8*arrayIndex;
    uint16_t eepromAddress = EEPROM_BUTTONS_TYPE_START+arrayIndex;

    bitWrite(_buttonType[arrayIndex], buttonIndex, type);
    eeprom_update_byte((uint8_t*)eepromAddress, _buttonType[arrayIndex]);

    setButtonPressed(buttonNumber, false);
    updateButtonState(buttonNumber, false);

    return (_buttonType[arrayIndex] == eeprom_read_byte((uint8_t*)eepromAddress));

}

bool OpenDeck::setButtonPCenabled(uint8_t buttonNumber, bool value)  {

    uint8_t arrayIndex = buttonNumber/8;
    uint8_t buttonIndex = buttonNumber - 8*arrayIndex;
    uint16_t eepromAddress = EEPROM_BUTTONS_PC_ENABLED_START+arrayIndex;

    bitWrite(buttonPCenabled[arrayIndex], buttonIndex, value);
    eeprom_update_byte((uint8_t*)eepromAddress, buttonPCenabled[arrayIndex]);

    setButtonPressed(buttonNumber, false);
    updateButtonState(buttonNumber, false);

    return (buttonPCenabled[arrayIndex] == eeprom_read_byte((uint8_t*)eepromAddress));

}

bool OpenDeck::setButtonNote(uint8_t buttonNumber, uint8_t _buttonNote)    {

    uint16_t eepromAddress = EEPROM_BUTTONS_NOTE_START+buttonNumber;

    noteNumber[buttonNumber] = _buttonNote;
    eeprom_update_byte((uint8_t*)eepromAddress, _buttonNote);
    return (_buttonNote == eeprom_read_byte((uint8_t*)eepromAddress));

}

bool OpenDeck::setLEDHwParameter(uint8_t parameter, uint8_t value) {

    switch(parameter)   {

        case SYS_EX_LEDS_HW_P_TOTAL_NUMBER:
        //set total number of LEDs (needed for start-up routine)
        totalNumberOfLEDs = value;
        eeprom_update_byte((uint8_t*)EEPROM_LEDS_HW_P_TOTAL_NUMBER, value);
        return (eeprom_read_byte((uint8_t*)EEPROM_LEDS_HW_P_TOTAL_NUMBER) == value);
        break;

        case SYS_EX_LEDS_HW_P_BLINK_TIME:
        //blink time
        boardObject.resetLEDblinkCounter();
        boardObject.setLEDblinkTime(value*100);
        eeprom_update_byte((uint8_t*)EEPROM_LEDS_HW_P_BLINK_TIME, value);
        return (eeprom_read_byte((uint8_t*)EEPROM_LEDS_HW_P_BLINK_TIME) == value);
        break;

        case SYS_EX_LEDS_HW_P_START_UP_SWITCH_TIME:
        //start-up led switch time
        eeprom_update_byte((uint8_t*)EEPROM_LEDS_HW_P_START_UP_SWITCH_TIME, value);
        return (eeprom_read_byte((uint8_t*)EEPROM_LEDS_HW_P_START_UP_SWITCH_TIME) == value);
        break;

        case SYS_EX_LEDS_HW_P_START_UP_ROUTINE:
        //set start-up routine pattern
        eeprom_update_byte((uint8_t*)EEPROM_LEDS_HW_P_START_UP_ROUTINE, value);
        return (eeprom_read_byte((uint8_t*)EEPROM_LEDS_HW_P_START_UP_ROUTINE) == value);
        break;

        case SYS_EX_LEDS_HW_P_FADE_TIME:
        boardObject.resetLEDtransitions();
        eeprom_update_byte((uint8_t*)EEPROM_LEDS_HW_P_FADE_SPEED, value);
        if (eeprom_read_byte((uint8_t*)EEPROM_LEDS_HW_P_FADE_SPEED) == value) {

            boardObject.setLEDTransitionSpeed(value);
            return true;

        } return false;
        break;

    }   return false;

}

bool OpenDeck::setAnalogEnabled(uint8_t analogID, bool state)    {

    uint8_t arrayIndex = analogID/8;
    uint8_t analogIndex = analogID - 8*arrayIndex;
    uint16_t eepromAddress = EEPROM_ANALOG_ENABLED_START+arrayIndex;

    bitWrite(analogEnabled[arrayIndex], analogIndex, state);
    eeprom_update_byte((uint8_t*)eepromAddress, analogEnabled[arrayIndex]);

    return (analogEnabled[arrayIndex] == eeprom_read_byte((uint8_t*)eepromAddress));

}

bool OpenDeck::setAnalogType(uint8_t analogID, uint8_t type)    {

    uint16_t eepromAddress = EEPROM_ANALOG_TYPE_START+analogID;
    analogType[analogID] = type;
    eeprom_update_byte((uint8_t*)eepromAddress, analogType[analogID]);

    return (analogType[analogID] == eeprom_read_byte((uint8_t*)eepromAddress));
}

bool OpenDeck::setAnalogInvertState(uint8_t analogID, bool state)    {

    uint8_t arrayIndex = analogID/8;
    uint8_t analogIndex = analogID - 8*arrayIndex;
    uint16_t eepromAddress = EEPROM_ANALOG_INVERTED_START+arrayIndex;

    bitWrite(analogInverted[arrayIndex], analogIndex, state);
    eeprom_update_byte((uint8_t*)eepromAddress, analogInverted[arrayIndex]);

    return (analogInverted[arrayIndex] == eeprom_read_byte((uint8_t*)eepromAddress));

}

bool OpenDeck::setCCnumber(uint8_t analogID, uint8_t _ccppNumber)   {

    uint16_t eepromAddress = EEPROM_ANALOG_NUMBER_START+analogID;

    ccNumber[analogID] = _ccppNumber;
    eeprom_update_byte((uint8_t*)eepromAddress, _ccppNumber);
    return (_ccppNumber == eeprom_read_byte((uint8_t*)eepromAddress));

}

bool OpenDeck::setAnalogLimit(uint8_t limitType, uint8_t _ccNumber, uint8_t newLimit)  {

    switch (limitType)  {

        case SYS_EX_MST_ANALOG_LOWER_LIMIT:
        analogLowerLimit[_ccNumber] = newLimit;
        eeprom_update_byte((uint8_t*)EEPROM_ANALOG_LOWER_LIMIT_START+_ccNumber, newLimit);
        return (eeprom_read_byte((uint8_t*)EEPROM_ANALOG_LOWER_LIMIT_START+_ccNumber) == newLimit);
        break;

        case SYS_EX_MST_ANALOG_UPPER_LIMIT:
        analogUpperLimit[_ccNumber] = newLimit;
        eeprom_update_byte((uint8_t*)EEPROM_ANALOG_UPPER_LIMIT_START+_ccNumber, newLimit);
        return (eeprom_read_byte((uint8_t*)EEPROM_ANALOG_UPPER_LIMIT_START+_ccNumber) == newLimit);
        break;

        default:
        return false;
        break;

    }

}

bool OpenDeck::setEncoderEnabled(uint8_t encoderID, bool state)    {

    uint8_t arrayIndex = encoderID >> 3;
    uint8_t encoderIndex = encoderID - 8*arrayIndex;
    uint16_t eepromAddress = EEPROM_ENCODERS_ENABLED_START+arrayIndex;

    bitWrite(encoderEnabled[arrayIndex], encoderIndex, state);
    eeprom_update_byte((uint8_t*)eepromAddress, encoderEnabled[arrayIndex]);

    return (encoderEnabled[arrayIndex] == eeprom_read_byte((uint8_t*)eepromAddress));

}

bool OpenDeck::setEncoderInvertState(uint8_t encoderID, bool state)    {

    uint8_t arrayIndex = encoderID >> 3;
    uint8_t encoderIndex = encoderID - 8*arrayIndex;
    uint16_t eepromAddress = EEPROM_ENCODERS_INVERTED_START+arrayIndex;

    bitWrite(encoderInverted[arrayIndex], encoderIndex, state);
    eeprom_update_byte((uint8_t*)eepromAddress, encoderInverted[arrayIndex]);

    return (encoderInverted[arrayIndex] == eeprom_read_byte((uint8_t*)eepromAddress));

}

bool OpenDeck::setEncoderNumber(uint8_t encoder, uint8_t number)   {

    uint16_t eepromAddress = EEPROM_ENCODERS_NUMBER_START+encoder;

    encoderNumber[encoder] = number;
    eeprom_update_byte((uint8_t*)eepromAddress, number);
    return (number == eeprom_read_byte((uint8_t*)eepromAddress));

}

bool OpenDeck::setEncoderPulsesPerStep(uint8_t encoder, uint8_t pulses)   {

    //reset variables
    resetEncoderValues(encoder);

    uint16_t eepromAddress = EEPROM_ENCODERS_PULSES_PER_STEP_START+encoder;

    pulsesPerStep[encoder] = pulses;
    eeprom_update_byte((uint8_t*)eepromAddress, pulses);
    return (pulses == eeprom_read_byte((uint8_t*)eepromAddress));

}

bool OpenDeck::setEncoderFastMode(uint8_t encoder, bool state)    {

    //reset variables
    resetEncoderValues(encoder);

    uint8_t arrayIndex = encoder/8;
    uint8_t encoderIndex = encoder - 8*arrayIndex;
    uint16_t eepromAddress = EEPROM_ENCODERS_FAST_MODE_START+arrayIndex;

    bitWrite(encoderFastMode[arrayIndex], encoderIndex, state);
    eeprom_update_byte((uint8_t*)eepromAddress, encoderFastMode[arrayIndex]);

    return (encoderFastMode[arrayIndex] == eeprom_read_byte((uint8_t*)eepromAddress));

}

bool OpenDeck::setLEDActivationNote(uint8_t ledNumber, uint8_t _ledActNote) {

    uint16_t eepromAddress = EEPROM_LEDS_ACT_NOTE_START+ledNumber;

    ledActNote[ledNumber] = _ledActNote;
    //TODO: add check of same values
    eeprom_update_byte((uint8_t*)eepromAddress, _ledActNote);
    return _ledActNote == eeprom_read_byte((uint8_t*)eepromAddress);

}

bool OpenDeck::setLEDstartNumber(uint8_t startNumber, uint8_t ledNumber) {

    uint16_t eepromAddress = EEPROM_LEDS_START_UP_NUMBER_START+startNumber;
    eeprom_update_byte((uint8_t*)eepromAddress, ledNumber);
    return ledNumber == eeprom_read_byte((uint8_t*)eepromAddress);

}

bool OpenDeck::sysExSetDefaultConf()    {

    //erase entire eeprom first

    //write default configuration stored in PROGMEM to EEPROM
    for (int i=0; i<(int16_t)sizeof(defConf); i++)  {

        eeprom_update_byte((uint8_t*)i, pgm_read_byte(&(defConf[i])));
        if (!(eeprom_read_byte((uint8_t*)i) == (pgm_read_byte(&(defConf[i])))))
            return false;

    }

    getConfiguration();
    return true;

}

void OpenDeck::sendSysEx(uint8_t *sysExArray, uint8_t size)   {

    #ifdef USBMIDI
        uint8_t usbArraySize = size+2;
        uint8_t usbSysExArray[usbArraySize];
    #endif

    switch (sysExMessageSource) {

        case midiSource:
        #ifdef HW_MIDI
            MIDI.sendSysEx(size, sysExArray, false);
        #endif
        break;

        case usbSource:
        #ifdef USBMIDI
            //usbMIDI.sendSysEx needs start and stop byte (F0 and F7)

            //init array
            for (int i=0; i<usbArraySize; i++) usbSysExArray[i] = 0;

            //append sysex start byte
            usbSysExArray[0] = 0xF0;

            //copy array
            for (int i=0; i<size; i++)
            usbSysExArray[i+1] = sysExArray[i];

            //append sysexstop byte
            usbSysExArray[size+1] = 0xF7;

            //send modified array
            usbMIDI.sendSysEx(usbArraySize, usbSysExArray);
        #endif
        break;

    }

}

void OpenDeck::setSysExSource(sysExSource source)   {

    sysExMessageSource = source;

}