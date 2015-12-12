#include "Encoders.h"
#include "..\midi\MIDI.h"
#include "eeprom/EEPROMsettings.h"
#include "sysex/ProtocolDefinitions.h"
#include "sysex/SysEx.h"
#include "BitManipulation.h"

#define ENCODER_VALUE_LEFT_7FH01H   127
#define ENCODER_VALUE_RIGHT_7FH01H  1

#define ENCODER_VALUE_LEFT_3FH41H   63
#define ENCODER_VALUE_RIGHT_3FH41H  65

typedef enum {

    encoderEnabledConf,
    encoderInvertedConf,
    encoderEncodingModeConf,
    encoderMIDIidConf,
    ENCODERS_SUBTYPES

} sysExMessageSubTypeEncoders;

subtype encoderEnabledSubtype       = { MAX_NUMBER_OF_ENCODERS, 0, 1, EEPROM_ENCODERS_ENABLED_START, BIT_PARAMETER };
subtype encoderInvertedSubtype      = { MAX_NUMBER_OF_ENCODERS, 0, 1, EEPROM_ENCODERS_INVERTED_START, BIT_PARAMETER };
subtype encoderEncodingModeSubtype  = { MAX_NUMBER_OF_ENCODERS, 0, ENCODING_MODES-1, EEPROM_ENCODERS_ENCODING_MODE_START, BYTE_PARAMETER };
subtype encoderMIDIidSubtype        = { MAX_NUMBER_OF_ENCODERS, 0, 127, EEPROM_ENCODERS_NUMBER_START, BYTE_PARAMETER };

const subtype *encodersSubtypeArray[] = {

    &encoderEnabledSubtype,
    &encoderInvertedSubtype,
    &encoderEncodingModeSubtype,
    &encoderMIDIidSubtype

};

Encoders::Encoders()    {

    //def const

}

void Encoders::init()   {

    //define message for sysex configuration
    sysEx.addMessageType(SYS_EX_MT_ENCODERS, ENCODERS_SUBTYPES);

    for (int i=0; i<ENCODERS_SUBTYPES; i++)   {

        //define subtype messages
        sysEx.addMessageSubType(SYS_EX_MT_ENCODERS, i, encodersSubtypeArray[i]->parameters, encodersSubtypeArray[i]->lowValue, encodersSubtypeArray[i]->highValue);

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

    return eepromSettings.readParameter(encodersSubtypeArray[encoderEnabledConf]->eepromAddress, encoderID, encodersSubtypeArray[encoderEnabledConf]->parameterType);

}

bool Encoders::getEncoderInvertState(uint8_t encoderID) {

    return eepromSettings.readParameter(encodersSubtypeArray[encoderInvertedConf]->eepromAddress, encoderID, encodersSubtypeArray[encoderInvertedConf]->parameterType);

}

encoderType Encoders::getEncoderType(uint8_t encoderID)  {

    return (encoderType)eepromSettings.readParameter(encodersSubtypeArray[encoderEncodingModeConf]->eepromAddress, encoderID, encodersSubtypeArray[encoderEncodingModeConf]->parameterType);

}

uint8_t Encoders::getMIDIid(uint8_t encoderID)  {

    return eepromSettings.readParameter(encodersSubtypeArray[encoderMIDIidConf]->eepromAddress, encoderID, encodersSubtypeArray[encoderMIDIidConf]->parameterType);

}

uint8_t Encoders::getParameter(uint8_t messageType, uint8_t parameterID)  {

    return eepromSettings.readParameter(encodersSubtypeArray[messageType]->eepromAddress, parameterID, encodersSubtypeArray[messageType]->parameterType);

}

bool Encoders::setEncoderEnabled(uint8_t encoderID, uint8_t state)    {

   return eepromSettings.writeParameter(encodersSubtypeArray[encoderEnabledConf]->eepromAddress, encoderID, state, encodersSubtypeArray[encoderEnabledConf]->parameterType);

}

bool Encoders::setEncoderInvertState(uint8_t encoderID, uint8_t state)    {

   return eepromSettings.writeParameter(encodersSubtypeArray[encoderInvertedConf]->eepromAddress, encoderID, state, encodersSubtypeArray[encoderInvertedConf]->parameterType);

}

bool Encoders::setEncoderType(uint8_t encoderID, uint8_t type)  {

   return eepromSettings.writeParameter(encodersSubtypeArray[encoderEncodingModeConf]->eepromAddress, encoderID, type, encodersSubtypeArray[encoderEncodingModeConf]->parameterType);

}

bool Encoders::setMIDIid(uint8_t encoderID, uint8_t midiID)  {

   return eepromSettings.writeParameter(encodersSubtypeArray[encoderMIDIidConf]->eepromAddress, encoderID, midiID, encodersSubtypeArray[encoderMIDIidConf]->parameterType);

}

bool Encoders::setParameter(uint8_t messageType, uint8_t parameter, uint8_t newParameter)   {

    return eepromSettings.writeParameter(encodersSubtypeArray[messageType]->eepromAddress, parameter, newParameter, encodersSubtypeArray[messageType]->parameterType);

}



Encoders encoders;