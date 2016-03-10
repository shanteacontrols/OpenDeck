#include "Analog.h"
#include "../../sysex/SysEx.h"
#include "../../eeprom/Blocks.h"

//use 1k resistor when connecting FSR between signal and ground

#define FSR_MIN_VALUE                       40
#define FSR_MAX_VALUE                       340

#define AFTERTOUCH_MAX_VALUE                600

#define FSR_MEDIAN_RUNS                     2

#define AFTERTOUCH_SEND_TIMEOUT_IGNORE      25       //ignore aftertouch reading change below this timeout

#define AFTERTOUCH_SEND_TIMEOUT_STEP        2

#define AFTERTOUCH_SEND_TIMEOUT             100

#define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))

inline int16_t mapAnalog_int16(int16_t x, int16_t in_min, int16_t in_max, int16_t out_min, int16_t out_max) {

    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;

};

inline int16_t calibratePressure(int16_t value, pressureType type)  {

    switch(type)    {

        case velocity:
        return mapAnalog_int16(constrain(value, FSR_MIN_VALUE, FSR_MAX_VALUE), FSR_MIN_VALUE, FSR_MAX_VALUE, 0, 127);

        case aftertouch:
        return mapAnalog_int16(constrain(value, FSR_MIN_VALUE, AFTERTOUCH_MAX_VALUE), FSR_MIN_VALUE, AFTERTOUCH_MAX_VALUE, 0, 127);

        default:
        return 0;

    }

}

bool Analog::getFsrPressed(uint8_t fsrID)   {

    uint8_t arrayIndex = fsrID/8;
    uint8_t fsrIndex = fsrID - 8*arrayIndex;

    return bitRead(fsrPressed[arrayIndex], fsrIndex);

}

void Analog::setFsrPressed(uint8_t fsrID, bool state)   {

    uint8_t arrayIndex = fsrID/8;
    uint8_t fsrIndex = fsrID - 8*arrayIndex;

    bitWrite(fsrPressed[arrayIndex], fsrIndex, state);

}

void Analog::checkFSRvalue(uint8_t analogID, int16_t pressure)  {

    uint8_t calibratedPressure = calibratePressure(pressure, velocity);

    fsrMedianRunCounter[analogID]++;
    lastAnalogueValue[analogID] += calibratedPressure;

    if (fsrMedianRunCounter[analogID] == FSR_MEDIAN_RUNS) {

        fsrMedianRunCounter[analogID] = 0;
        calibratedPressure = lastAnalogueValue[analogID] / FSR_MEDIAN_RUNS;
        lastAnalogueValue[analogID] = 0;

    }   else return;

    bool pressDetected = (calibratedPressure > 0);

    switch (pressDetected)    {

        case true:

        if (!getFsrPressed(analogID)) {

            //sensor is really pressed
            setFsrPressed(analogID, true);
            midi.sendMIDInote(getMIDIid(analogID), true, calibratedPressure);
            if (sysEx.configurationEnabled()) sysEx.sendComponentID(CONF_ANALOG_BLOCK, analogID);

        }
        break;

        case false:

        if (getFsrPressed(analogID))  {

            setFsrPressed(analogID, false);
            midi.sendMIDInote(getMIDIid(analogID), false, 0);
            if (sysEx.configurationEnabled()) sysEx.sendComponentID(CONF_ANALOG_BLOCK, analogID);

        }

        break;

    }

}
