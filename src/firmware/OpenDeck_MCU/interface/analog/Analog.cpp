#include "Analog.h"
#include "..\sysex\SysEx.h"
#include "..\eeprom\Configuration.h"
#include "..\interface\settings\AnalogSettings.h"

Analog::Analog()    {

    //def const

}

void Analog::init() {

    const subtype analogEnabledSubtype       = { MAX_NUMBER_OF_ANALOG, 0, 1 };
    const subtype analogInvertedSubtype      = { MAX_NUMBER_OF_ANALOG, 0, 1 };
    const subtype analogTypeSubtype          = { MAX_NUMBER_OF_ANALOG, 0, ANALOG_TYPES-1 };
    const subtype analogMIDIidSubtype        = { MAX_NUMBER_OF_ANALOG, 0, 127 };
    const subtype analogCClowerLimitSubtype  = { MAX_NUMBER_OF_ANALOG, 0, 127 };
    const subtype analogCCupperLimitSubtype  = { MAX_NUMBER_OF_ANALOG, 0, 127 };

    const subtype *analogSubtypeArray[] = {

        &analogEnabledSubtype,
        &analogTypeSubtype,
        &analogInvertedSubtype,
        &analogMIDIidSubtype,
        &analogCClowerLimitSubtype,
        &analogCCupperLimitSubtype

    };

    //define message for sysex configuration
    sysEx.addMessageType(CONF_ENCODER_BLOCK, ANALOG_SUBTYPES);

    for (int i=0; i<ANALOG_SUBTYPES; i++)   {

        //define subtype messages
        sysEx.addMessageSubType(CONF_ENCODER_BLOCK, i, analogSubtypeArray[i]->parameters, analogSubtypeArray[i]->lowValue, analogSubtypeArray[i]->highValue);

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
            checkPotentiometerValue(i, analogData);
            break;

            case fsr:
            checkFSRvalue(i, analogData);
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

    return configuration.readParameter(CONF_ANALOG_BLOCK, analogEnabledSection, analogID);

}

bool Analog::getAnalogInvertState(uint8_t analogID) {

    return configuration.readParameter(CONF_ANALOG_BLOCK, analogInvertedSection, analogID);

}

analogType Analog::getAnalogType(uint8_t analogID) {

    return (analogType)configuration.readParameter(CONF_ANALOG_BLOCK, analogTypeSection, analogID);

}

uint8_t Analog::getMIDIid(uint8_t analogID)    {

    return configuration.readParameter(CONF_ANALOG_BLOCK, analogMIDIidSection, analogID);

}

uint8_t Analog::getCClimit(uint8_t analogID, ccLimitType type)  {

    switch(type)    {

        case ccLimitLow:
        return configuration.readParameter(CONF_ANALOG_BLOCK, analogCClowerLimitSection, analogID);
        break;

        case ccLimitHigh:
        return configuration.readParameter(CONF_ANALOG_BLOCK, analogCCupperLimitSection, analogID);
        break;

    }   return 0;

}

uint8_t Analog::getParameter(uint8_t messageType, uint8_t parameter) {

    switch(messageType) {

        case analogEnabledConf:
        return getAnalogEnabled(parameter);
        break;

        case analogInvertedConf:
        return getAnalogInvertState(parameter);
        break;

        case analogTypeConf:
        return getAnalogType(parameter);
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

    }   return 0;

}


bool Analog::setAnalogEnabled(uint8_t analogID, bool state)    {

    return configuration.writeParameter(CONF_ANALOG_BLOCK, analogEnabledSection, analogID, state);

}

bool Analog::setAnalogInvertState(uint8_t analogID, uint8_t state) {

    return configuration.writeParameter(CONF_ANALOG_BLOCK, analogInvertedSection, analogID, state);

}

bool Analog::setAnalogType(uint8_t analogID, uint8_t type)    {

    lastAnalogueValue[analogID] = 0;

    return configuration.writeParameter(CONF_ANALOG_BLOCK, analogTypeSection, analogID, type);

}

bool Analog::setMIDIid(uint8_t analogID, uint8_t midiID)   {

    return configuration.writeParameter(CONF_ANALOG_BLOCK, analogMIDIidSection, analogID, midiID);

}

bool Analog::setCClimit(ccLimitType type, uint8_t analogID, uint8_t limit)  {

    switch (limit)  {

        case ccLimitLow:
        return configuration.writeParameter(CONF_ANALOG_BLOCK, analogCClowerLimitSection, analogID, limit);
        break;

        case ccLimitHigh:
        return configuration.writeParameter(CONF_ANALOG_BLOCK, analogCCupperLimitSection, analogID, limit);
        break;

        default:
        return false;
        break;

    }

}

bool Analog::setParameter(uint8_t messageType, uint8_t parameter, uint8_t newParameter)    {

    switch(messageType) {

        case analogEnabledConf:
        return setAnalogEnabled(parameter, newParameter);
        break;

        case analogInvertedConf:
        return setAnalogInvertState(parameter, newParameter);
        break;

        case analogTypeConf:
        return setAnalogType(parameter, newParameter);
        break;

        case analogMIDIidConf:
        return setMIDIid(parameter, newParameter);
        break;

        case analogCClowerLimitConf:
        return setCClimit(ccLimitLow, parameter, newParameter);
        break;

        case analogCCupperLimitConf:
        return setCClimit(ccLimitLow, parameter, newParameter);
        break;

    }   return false;

}

Analog analog;