#include "Encoders.h"
#include "..\midi\MIDI.h"
#include "eeprom/EEPROMsettings.h"
#include "sysex/SysEx.h"
#include "BitManipulation.h"

typedef enum {

    encoderEnabledConf,
    encoderInvertedConf,
    encoderEncodingModeConf,
    encoderMIDIidConf,
    ENCODER_SUBTYPES

} sysExMessageSubTypeEncoders;

#define ENCODER_VALUE_LEFT_7FH01H   127
#define ENCODER_VALUE_RIGHT_7FH01H  1

#define ENCODER_VALUE_LEFT_3FH41H   63
#define ENCODER_VALUE_RIGHT_3FH41H  65

Encoders::Encoders()    {

    //def const

}

void Encoders::init()   {

    const subtype encoderEnabledSubtype       = { MAX_NUMBER_OF_ENCODERS, 0, 1 };
    const subtype encoderInvertedSubtype      = { MAX_NUMBER_OF_ENCODERS, 0, 1 };
    const subtype encoderEncodingModeSubtype  = { MAX_NUMBER_OF_ENCODERS, 0, ENCODING_MODES-1 };
    const subtype encoderMIDIidSubtype        = { MAX_NUMBER_OF_ENCODERS, 0, 127 };

    const subtype *encodersSubtypeArray[] = {

        &encoderEnabledSubtype,
        &encoderInvertedSubtype,
        &encoderEncodingModeSubtype,
        &encoderMIDIidSubtype

    };

    //define message for sysex configuration
    sysEx.addMessageType(CONF_ANALOG_BLOCK, ENCODER_SUBTYPES);

    for (int i=0; i<ENCODER_SUBTYPES; i++)   {

        //define subtype messages
        sysEx.addMessageSubType(CONF_ANALOG_BLOCK, i, encodersSubtypeArray[i]->parameters, encodersSubtypeArray[i]->lowValue, encodersSubtypeArray[i]->highValue);

    }

}

void Encoders::update()   {

    if (!board.encoderDataAvailable()) return;

    for (int i=0; i<MAX_NUMBER_OF_ENCODERS; i++)    {

        if (!getEncoderEnabled(i)) continue;

        encoderPosition encoderState = board.getEncoderState(i);
        if (encoderState == encStopped) continue;

        if (getEncoderInvertState(i))   {

            if (encoderState == encMoveLeft)
                encoderState = encMoveRight;

             else encoderState = encMoveLeft;

        }

        uint8_t encoderValue = 0;

        switch(getEncoderType(i)) {

            case enc7Fh01h:
            if (encoderState == encMoveLeft) encoderValue = ENCODER_VALUE_LEFT_7FH01H;
            else encoderValue = ENCODER_VALUE_RIGHT_7FH01H;
            break;

            case enc3Fh41h:
            if (encoderState == encMoveLeft) encoderValue = ENCODER_VALUE_LEFT_3FH41H;
            else encoderValue = ENCODER_VALUE_RIGHT_3FH41H;
            break;

            default:
            break;

        }

        midi.sendControlChange(getMIDIid(i), encoderValue);
        if (sysEx.configurationEnabled()) sysEx.sendID(encoderComponent, i);

    }

}

bool Encoders::getEncoderEnabled(uint8_t encoderID) {

    return eepromSettings.readParameter(EEPROM_ENCODERS_ENABLED_START, encoderID, BIT_PARAMETER);

}

bool Encoders::getEncoderInvertState(uint8_t encoderID) {

    return eepromSettings.readParameter(EEPROM_ENCODERS_INVERTED_START, encoderID, BIT_PARAMETER);

}

encoderType Encoders::getEncoderType(uint8_t encoderID)  {

    return (encoderType)eepromSettings.readParameter(EEPROM_ENCODERS_ENCODING_MODE_START, encoderID, BYTE_PARAMETER);

}

uint8_t Encoders::getMIDIid(uint8_t encoderID)  {

    return eepromSettings.readParameter(EEPROM_ENCODERS_NUMBER_START, encoderID, BYTE_PARAMETER);

}

uint8_t Encoders::getParameter(uint8_t messageType, uint8_t parameterID)  {

    switch(messageType) {

        case encoderEnabledConf:
        return getEncoderEnabled(parameterID);
        break;

        case encoderInvertedConf:
        return getEncoderInvertState(parameterID);
        break;

        case encoderEncodingModeConf:
        return getEncoderType(parameterID);
        break;

        case encoderMIDIidConf:
        return getMIDIid(parameterID);
        break;

    }   return 0;

}

bool Encoders::setEncoderEnabled(uint8_t encoderID, uint8_t state)    {

   return eepromSettings.writeParameter(EEPROM_ENCODERS_ENABLED_START, encoderID, state, BIT_PARAMETER);

}

bool Encoders::setEncoderInvertState(uint8_t encoderID, uint8_t state)    {

   return eepromSettings.writeParameter(EEPROM_ENCODERS_INVERTED_START, encoderID, state, BIT_PARAMETER);

}

bool Encoders::setEncoderType(uint8_t encoderID, uint8_t type)  {

   return eepromSettings.writeParameter(EEPROM_ENCODERS_ENCODING_MODE_START, encoderID, type, BYTE_PARAMETER);

}

bool Encoders::setMIDIid(uint8_t encoderID, uint8_t midiID)  {

   return eepromSettings.writeParameter(EEPROM_ENCODERS_NUMBER_START, encoderID, midiID, BYTE_PARAMETER);

}

bool Encoders::setParameter(uint8_t messageType, uint8_t parameter, uint8_t newParameter)   {

    switch(messageType) {

        case encoderEnabledConf:
        return setEncoderEnabled(parameter, newParameter);
        break;

        case encoderInvertedConf:
        return setEncoderInvertState(parameter, newParameter);
        break;

        case encoderEncodingModeConf:
        return setEncoderType(parameter, newParameter);
        break;

        case encoderMIDIidConf:
        return setMIDIid(parameter, newParameter);
        break;

    }   return 0;

}



Encoders encoders;