#include "Analog.h"
#include "sysex/SysEx.h"
#include "eeprom/EEPROMsettings.h"
#include "sysex/ProtocolDefinitions.h"
#include "..\midi\MIDI.h"
#include "BitManipulation.h"

typedef enum {

    analogEnabledConf,
    analogTypeConf,
    analogInvertedConf,
    analogMIDIidConf,
    analogCClowerLimitConf,
    analogCCupperLimitConf,
    ANALOG_SUBTYPES

} sysExMessageSubtypeAnalog;

subtype analogEnabledSubtype       = { MAX_NUMBER_OF_ANALOG, 0, 1 };
subtype analogTypeSubtype          = { MAX_NUMBER_OF_ANALOG, 0, ANALOG_TYPES-1 };
subtype analogInvertedSubtype      = { MAX_NUMBER_OF_ANALOG, 0, 1 };
subtype analogMIDIidSubtype        = { MAX_NUMBER_OF_ANALOG, 0, 127 };
subtype analogCClowerLimitSubtype  = { MAX_NUMBER_OF_ANALOG, 0, 127 };
subtype analogCCupperLimitSubtype  = { MAX_NUMBER_OF_ANALOG, 0, 127 };

const uint8_t analogSubtypeArray[] = {

    analogEnabledConf,
    analogTypeConf,
    analogInvertedConf,
    analogMIDIidConf,
    analogCClowerLimitConf,
    analogCCupperLimitConf

};

const uint8_t analogParametersArray[] = {

    analogEnabledSubtype.parameters,
    analogTypeSubtype.parameters,
    analogInvertedSubtype.parameters,
    analogMIDIidSubtype.parameters,
    analogCClowerLimitSubtype.parameters,
    analogCCupperLimitSubtype.parameters,

};

const uint8_t analogNewParameterLowArray[] = {

    analogEnabledSubtype.lowValue,
    analogTypeSubtype.lowValue,
    analogInvertedSubtype.lowValue,
    analogMIDIidSubtype.lowValue,
    analogCClowerLimitSubtype.lowValue,
    analogCCupperLimitSubtype.lowValue

};

const uint8_t analogNewParameterHighArray[] = {

    analogEnabledSubtype.highValue,
    analogTypeSubtype.highValue,
    analogInvertedSubtype.highValue,
    analogMIDIidSubtype.highValue,
    analogCClowerLimitSubtype.highValue,
    analogCCupperLimitSubtype.highValue

};

Analog::Analog()    {

    //def const

}

void Analog::init() {

    //define message for sysex configuration
    sysEx.addMessageType(SYS_EX_MT_ANALOG, ANALOG_SUBTYPES);

    for (int i=0; i<ANALOG_SUBTYPES; i++)   {

        //define subtype messages
        sysEx.addMessageSubType(SYS_EX_MT_ANALOG, analogSubtypeArray[i], analogParametersArray[i], analogNewParameterLowArray[i], analogNewParameterHighArray[i]);

    }

}

void Analog::update()   {

    if (!board.analogDataAvailable()) return;

    int16_t analogData;

    //check values
    for (int i=0; i<MAX_NUMBER_OF_ANALOG; i++)    {

        analogData = board.getAnalogValue(i); //get raw analog reading
        if (!getAnalogEnabled(i)) continue; //don't process component if it's not enabled
        addAnalogSample(i, analogData);
        if (!analogValueSampled(i)) continue;  //three samples are needed
        analogData = getMedianValue(i);  //get median value from three analog samples for better accuracy
        analogType type = getAnalogType(i);

        switch(type) {

            case potentiometer:
            checkPotentiometerValue(analogData, i);
            break;

            case fsr:
            break;

            case ldr:
            break;

            default:
            return;

        }

    }

}



void Analog::addAnalogSample(uint8_t analogID, int16_t sample) {

    uint8_t sampleIndex = analogDebounceCounter[analogID];

    analogSample[analogID][sampleIndex] = sample;
    analogDebounceCounter[analogID]++;

}

bool Analog::analogValueSampled(uint8_t analogID) {

    if (analogDebounceCounter[analogID] == NUMBER_OF_SAMPLES) {

        analogDebounceCounter[analogID] = 0;
        return true;

    }   return false;

}

bool Analog::getAnalogEnabled(uint8_t analogID) {

    uint8_t arrayIndex = analogID/8;
    uint8_t analogIndex = analogID - 8*arrayIndex;
    uint8_t analogEnabledArray = eeprom_read_byte((uint8_t*)EEPROM_ANALOG_ENABLED_START+arrayIndex);

    return bitRead(analogEnabledArray, analogIndex);

}

analogType Analog::getAnalogType(uint8_t analogID) {

    return (analogType)eeprom_read_byte((uint8_t*)EEPROM_ANALOG_TYPE_START+analogID);

}

bool Analog::getAnalogInvertState(uint8_t analogID) {

    uint8_t arrayIndex = analogID/8;
    uint8_t analogIndex = analogID - 8*arrayIndex;
    uint8_t analogInvertedArray = eeprom_read_byte((uint8_t*)EEPROM_ANALOG_INVERTED_START+arrayIndex);

    return bitRead(analogInvertedArray, analogIndex);

}

uint8_t Analog::getMIDIid(uint8_t analogID)    {

    return eeprom_read_byte((uint8_t*)EEPROM_ANALOG_NUMBER_START+analogID);

}

int16_t Analog::getMedianValue(uint8_t analogID)  {

    int16_t medianValue = 0;

    if ((analogSample[analogID][0] <= analogSample[analogID][1]) && (analogSample[analogID][0] <= analogSample[analogID][2]))
    {
        medianValue = (analogSample[analogID][1] <= analogSample[analogID][2]) ? analogSample[analogID][1] : analogSample[analogID][2];
    }
    else if ((analogSample[analogID][1] <= analogSample[analogID][0]) && (analogSample[analogID][1] <= analogSample[analogID][2]))
    {
        medianValue = (analogSample[analogID][0] <= analogSample[analogID][2]) ? analogSample[analogID][0] : analogSample[analogID][2];
    }
    else
    {
        medianValue = (analogSample[analogID][0] <= analogSample[analogID][1]) ? analogSample[analogID][0] : analogSample[analogID][1];
    }

    return medianValue;

}

uint8_t Analog::mapAnalog(uint8_t x, uint8_t in_min, uint8_t in_max, uint8_t out_min, uint8_t out_max)    {

    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;

}

bool Analog::setAnalogEnabled(uint8_t analogID, uint8_t state)    {

    uint8_t arrayIndex = analogID/8;
    uint8_t analogIndex = analogID - 8*arrayIndex;
    uint16_t address = EEPROM_ANALOG_ENABLED_START+arrayIndex;
    uint8_t analogEnabledArray = eeprom_read_byte((uint8_t*)address);

    if (state == RESET_VALUE)   bitWrite(analogEnabledArray, analogIndex, bitRead(pgm_read_byte(&(defConf[address])), analogIndex));
    else                        bitWrite(analogEnabledArray, analogIndex, state);

    eeprom_update_byte((uint8_t*)address, analogEnabledArray);

    return (analogEnabledArray == eeprom_read_byte((uint8_t*)address));

}

bool Analog::setAnalogType(uint8_t analogID, uint8_t type)    {

    uint16_t address = EEPROM_ANALOG_TYPE_START+analogID;

    if (type == RESET_VALUE) type = pgm_read_byte(&(defConf[address]));

    eeprom_update_byte((uint8_t*)address, (uint8_t)type);

    return ((uint8_t)type == eeprom_read_byte((uint8_t*)address));

}

uint8_t Analog::getCClimit(uint8_t analogID, ccLimitType type)  {

    switch(type)    {

        case ccLimitLow:
        return eeprom_read_byte((uint8_t*)EEPROM_ANALOG_LOWER_LIMIT_START+analogID);
        break;

        case ccLimitHigh:
        return eeprom_read_byte((uint8_t*)EEPROM_ANALOG_UPPER_LIMIT_START+analogID);
        break;

    }   return 0;

}

uint8_t Analog::getParameter(uint8_t messageType, uint8_t parameter) {

    switch(messageType) {

        case analogEnabledConf:
        return getAnalogEnabled(parameter);
        break;

        case analogTypeConf:
        return getAnalogType(parameter);
        break;

        case analogInvertedConf:
        return getAnalogInvertState(parameter);
        break;

        case analogMIDIidConf:
        return getMIDIid(parameter);
        break;

        case analogCClowerLimitConf:
        return getCClimit(parameter, ccLimitLow);
        break;

        case analogCCupperLimitConf:
         return getCClimit(parameter, ccLimitHigh);
        break;

    }   return INVALID_VALUE;

}

bool Analog::setParameter(uint8_t messageType, uint8_t parameter, uint8_t newParameter)    {

    switch(messageType) {

        case analogEnabledConf:
        return setAnalogEnabled(parameter, newParameter);
        break;

        case analogTypeConf:
        return setAnalogType(parameter, (analogType)newParameter);
        break;

        case analogInvertedConf:
        return setAnalogInvertState(parameter, newParameter);
        break;

        case analogMIDIidConf:
        return setMIDIid(parameter, newParameter);
        break;

        case analogCClowerLimitConf:
        return setAnalogLimit(ccLimitLow, parameter, newParameter);
        break;

        case analogCCupperLimitConf:
        return setAnalogLimit(ccLimitHigh, parameter, newParameter);
        break;

    }   return false;

}

bool Analog::setMIDIid(uint8_t analogID, uint8_t midiID)   {

    uint16_t address = EEPROM_ANALOG_NUMBER_START+analogID;

    if (midiID == RESET_VALUE) midiID = pgm_read_byte(&(defConf[address]));

    eeprom_update_byte((uint8_t*)address, midiID);

    return (midiID == eeprom_read_byte((uint8_t*)address));

}

bool Analog::setAnalogLimit(ccLimitType type, uint8_t analogID, uint8_t limit)  {

    uint16_t address;

    switch (limit)  {

        case ccLimitLow:
        address = EEPROM_ANALOG_LOWER_LIMIT_START+analogID;
        if (limit == RESET_VALUE) limit = pgm_read_byte(&(defConf[address]));
        eeprom_update_byte((uint8_t*)address, limit);
        return (eeprom_read_byte((uint8_t*)address) == limit);
        break;

        case ccLimitHigh:
        address = EEPROM_ANALOG_UPPER_LIMIT_START+analogID;
        if (limit == RESET_VALUE) limit = pgm_read_byte(&(defConf[address]));
        eeprom_update_byte((uint8_t*)address, limit);
        return (eeprom_read_byte((uint8_t*)address) == limit);
        break;

        default:
        return false;
        break;

    }

}

bool Analog::setAnalogInvertState(uint8_t analogID, uint8_t state) {

    uint8_t arrayIndex = analogID/8;
    uint8_t analogIndex = analogID - 8*arrayIndex;
    uint16_t address = EEPROM_ANALOG_INVERTED_START+arrayIndex;
    uint8_t analogInvertedArray = eeprom_read_byte((uint8_t*)address);

    if (state == RESET_VALUE)   bitWrite(analogInvertedArray, analogIndex, bitRead(pgm_read_byte(&(defConf[address])), analogIndex));
    else                        bitWrite(analogInvertedArray, analogIndex, state);

    eeprom_update_byte((uint8_t*)address, analogInvertedArray);

    return (analogInvertedArray == eeprom_read_byte((uint8_t*)address));

}

Analog analog;