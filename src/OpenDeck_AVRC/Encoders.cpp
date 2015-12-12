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

subtype encoderEnabledSubtype       = { MAX_NUMBER_OF_ENCODERS, 0, 1 };
subtype encoderInvertedSubtype      = { MAX_NUMBER_OF_ENCODERS, 0, 1 };
subtype encoderEncodingModeSubtype  = { MAX_NUMBER_OF_ENCODERS, 0, ENCODING_MODES-1 };
subtype encoderMIDIidSubtype        = { MAX_NUMBER_OF_ENCODERS, 0, 127 };

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

    uint8_t arrayIndex = encoderID/8;
    uint8_t encoderIndex = encoderID - 8*arrayIndex;
    uint8_t encoderEnabledArray = eeprom_read_byte((uint8_t*)EEPROM_ENCODERS_ENABLED_START+arrayIndex);

    return bitRead(encoderEnabledArray, encoderIndex);

}

bool Encoders::getEncoderInvertState(uint8_t encoderID) {

    uint8_t arrayIndex = encoderID/8;
    uint8_t encoderIndex = encoderID - 8*arrayIndex;
    uint8_t encoderInvertedArray = eeprom_read_byte((uint8_t*)EEPROM_ENCODERS_INVERTED_START+arrayIndex);

    return bitRead(encoderInvertedArray, encoderIndex);

}

encoderType Encoders::getEncoderType(uint8_t encoderID)  {

    uint8_t returnValue = eeprom_read_byte((uint8_t*)EEPROM_ENCODERS_ENCODING_MODE_START+encoderID);
    return (encoderType)returnValue;

}

bool Encoders::setEncoderEnabled(uint8_t encoderID, uint8_t state)    {

   uint8_t arrayIndex = encoderID/8;
   uint8_t encoderIndex = encoderID - 8*arrayIndex;
   uint16_t address = EEPROM_ENCODERS_ENABLED_START+arrayIndex;
   uint8_t encoderEnabledArray = eeprom_read_byte((uint8_t*)address);

   if (state == RESET_VALUE)   bitWrite(encoderEnabledArray, encoderIndex, bitRead(pgm_read_byte(&(defConf[address])), encoderIndex));
   else                        bitWrite(encoderEnabledArray, encoderIndex, state);

   eeprom_update_byte((uint8_t*)address, encoderEnabledArray);

   return (encoderEnabledArray == eeprom_read_byte((uint8_t*)address));

}

bool Encoders::setEncoderInvertState(uint8_t encoderID, uint8_t state)    {

    uint8_t arrayIndex = encoderID/8;
    uint8_t encoderIndex = encoderID - 8*arrayIndex;
    uint16_t address = EEPROM_ENCODERS_INVERTED_START+arrayIndex;
    uint8_t encoderInvertedArray = eeprom_read_byte((uint8_t*)address);

    if (state == RESET_VALUE)   bitWrite(encoderInvertedArray, encoderIndex, bitRead(pgm_read_byte(&(defConf[address])), encoderIndex));
    else                        bitWrite(encoderInvertedArray, encoderIndex, state);

    eeprom_update_byte((uint8_t*)address, encoderInvertedArray);

    return (encoderInvertedArray == eeprom_read_byte((uint8_t*)address));

}

bool Encoders::setEncoderType(uint8_t encoderID, uint8_t type)  {

    int16_t address = EEPROM_ENCODERS_ENCODING_MODE_START+encoderID;

    if (type == RESET_VALUE) type = pgm_read_byte(&(defConf[address]));

    eeprom_update_byte((uint8_t*)address, (uint8_t)type);
    return (type == eeprom_read_byte((uint8_t*)address));

}

uint8_t Encoders::getParameter(uint8_t messageType, uint8_t parameter)  {

    switch(messageType) {

        case encoderEnabledConf:
        return getEncoderEnabled(parameter);
        break;

        case encoderInvertedConf:
        return getEncoderInvertState(parameter);
        break;

        case encoderEncodingModeConf:
        return getEncoderType(parameter);
        break;

        case encoderMIDIidConf:
        return getMIDIid(parameter);
        break;

        default:
        break;

    }   return INVALID_VALUE;

}

uint8_t Encoders::getMIDIid(uint8_t encoderID)  {

    return eeprom_read_byte((uint8_t*)EEPROM_ENCODERS_NUMBER_START+encoderID);

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

    }   return false;

}

bool Encoders::setMIDIid(uint8_t encoderID, uint8_t midiID)  {

    uint16_t address = EEPROM_ENCODERS_NUMBER_START+encoderID;

    if (midiID == RESET_VALUE) midiID = pgm_read_byte(&(defConf[address]));

    eeprom_update_byte((uint8_t*)address, midiID);
    return (midiID == eeprom_read_byte((uint8_t*)address));

}

Encoders encoders;