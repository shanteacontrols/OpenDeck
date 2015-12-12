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

const subtype analogEnabledSubtype       = { MAX_NUMBER_OF_ANALOG, 0, 1, EEPROM_ANALOG_ENABLED_START, BIT_PARAMETER };
const subtype analogInvertedSubtype      = { MAX_NUMBER_OF_ANALOG, 0, 1, EEPROM_ANALOG_INVERTED_START, BIT_PARAMETER };
const subtype analogTypeSubtype          = { MAX_NUMBER_OF_ANALOG, 0, ANALOG_TYPES-1, EEPROM_ANALOG_TYPE_START, BYTE_PARAMETER };
const subtype analogMIDIidSubtype        = { MAX_NUMBER_OF_ANALOG, 0, 127, EEPROM_ANALOG_NUMBER_START, BYTE_PARAMETER };
const subtype analogCClowerLimitSubtype  = { MAX_NUMBER_OF_ANALOG, 0, 127, EEPROM_ANALOG_LOWER_LIMIT_START, BYTE_PARAMETER };
const subtype analogCCupperLimitSubtype  = { MAX_NUMBER_OF_ANALOG, 0, 127, EEPROM_ANALOG_UPPER_LIMIT_START, BYTE_PARAMETER };

const subtype *analogSubtypeArray[] = {

    &analogEnabledSubtype,
    &analogTypeSubtype,
    &analogInvertedSubtype,
    &analogMIDIidSubtype,
    &analogCClowerLimitSubtype,
    &analogCCupperLimitSubtype

};

Analog::Analog()    {

    //def const

}

void Analog::init() {

    //define message for sysex configuration
    sysEx.addMessageType(SYS_EX_MT_ANALOG, ANALOG_SUBTYPES);

    for (int i=0; i<ANALOG_SUBTYPES; i++)   {

        //define subtype messages
        sysEx.addMessageSubType(SYS_EX_MT_ANALOG, i, analogSubtypeArray[i]->parameters, analogSubtypeArray[i]->lowValue, analogSubtypeArray[i]->highValue);

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

int16_t Analog::getMedianValue(uint8_t analogID)  {

    int16_t medianValue = 0;

    if ((analogSample[analogID][0] <= analogSample[analogID][1]) && (analogSample[analogID][0] <= analogSample[analogID][2]))
    medianValue = (analogSample[analogID][1] <= analogSample[analogID][2]) ? analogSample[analogID][1] : analogSample[analogID][2];

    else if ((analogSample[analogID][1] <= analogSample[analogID][0]) && (analogSample[analogID][1] <= analogSample[analogID][2]))
    medianValue = (analogSample[analogID][0] <= analogSample[analogID][2]) ? analogSample[analogID][0] : analogSample[analogID][2];

    else
    medianValue = (analogSample[analogID][0] <= analogSample[analogID][1]) ? analogSample[analogID][0] : analogSample[analogID][1];


    return medianValue;

}


bool Analog::getAnalogEnabled(uint8_t analogID) {

    return eepromSettings.readParameter(analogSubtypeArray[analogEnabledConf]->eepromAddress, analogID, analogSubtypeArray[analogEnabledConf]->parameterType);

}

bool Analog::getAnalogInvertState(uint8_t analogID) {

    return eepromSettings.readParameter(analogSubtypeArray[analogInvertedConf]->eepromAddress, analogID, analogSubtypeArray[analogInvertedConf]->parameterType);

}

analogType Analog::getAnalogType(uint8_t analogID) {

    return (analogType)eepromSettings.readParameter(analogSubtypeArray[analogTypeConf]->eepromAddress, analogID, analogSubtypeArray[analogTypeConf]->parameterType);

}

uint8_t Analog::getMIDIid(uint8_t analogID)    {

    return eepromSettings.readParameter(analogSubtypeArray[analogMIDIidConf]->eepromAddress, analogID, analogSubtypeArray[analogMIDIidConf]->parameterType);

}

uint8_t Analog::getCClimit(uint8_t analogID, ccLimitType type)  {

    switch(type)    {

        case ccLimitLow:
        return eepromSettings.readParameter(analogSubtypeArray[analogCClowerLimitConf]->eepromAddress, analogID, analogSubtypeArray[analogCClowerLimitConf]->parameterType);
        break;

        case ccLimitHigh:
        return eepromSettings.readParameter(analogSubtypeArray[analogCCupperLimitConf]->eepromAddress, analogID, analogSubtypeArray[analogCCupperLimitConf]->parameterType);
        break;

    }   return 0;

}

uint8_t Analog::getParameter(uint8_t messageType, uint8_t parameter) {

    return eepromSettings.readParameter(analogSubtypeArray[messageType]->eepromAddress, parameter, analogSubtypeArray[messageType]->parameterType);

}


bool Analog::setAnalogEnabled(uint8_t analogID, uint8_t state)    {

    return eepromSettings.writeParameter(analogSubtypeArray[analogEnabledConf]->eepromAddress, analogID, state, analogSubtypeArray[analogEnabledConf]->parameterType);

}

bool Analog::setAnalogInvertState(uint8_t analogID, uint8_t state) {

    return eepromSettings.writeParameter(analogSubtypeArray[analogInvertedConf]->eepromAddress, analogID, state, analogSubtypeArray[analogInvertedConf]->parameterType);

}

bool Analog::setAnalogType(uint8_t analogID, uint8_t type)    {

    return eepromSettings.writeParameter(analogSubtypeArray[analogTypeConf]->eepromAddress, analogID, type, analogSubtypeArray[analogTypeConf]->parameterType);

}

bool Analog::setMIDIid(uint8_t analogID, uint8_t midiID)   {

    return eepromSettings.writeParameter(analogSubtypeArray[analogMIDIidConf]->eepromAddress, analogID, midiID, analogSubtypeArray[analogMIDIidConf]->parameterType);

}

bool Analog::setAnalogLimit(ccLimitType type, uint8_t analogID, uint8_t limit)  {

    switch (limit)  {

        case ccLimitLow:
        return eepromSettings.writeParameter(analogSubtypeArray[analogCClowerLimitConf]->eepromAddress, analogID, limit, analogSubtypeArray[analogCClowerLimitConf]->parameterType);
        break;

        case ccLimitHigh:
        return eepromSettings.writeParameter(analogSubtypeArray[analogCCupperLimitConf]->eepromAddress, analogID, limit, analogSubtypeArray[analogCCupperLimitConf]->parameterType);
        break;

        default:
        return false;
        break;

    }

}

bool Analog::setParameter(uint8_t messageType, uint8_t parameter, uint8_t newParameter)    {

    return eepromSettings.writeParameter(analogSubtypeArray[messageType]->eepromAddress, parameter, newParameter, analogSubtypeArray[messageType]->parameterType);

}

Analog analog;