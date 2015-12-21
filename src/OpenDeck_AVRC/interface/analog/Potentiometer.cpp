#include "Analog.h"
#include "..\sysex/SysEx.h"

//potentiometer must exceed this value before sending new value
#define POTENTIOMETER_CC_STEP 8

void Analog::checkPotentiometerValue(int16_t tempValue, uint8_t analogID)  {

    //calculate difference between current and previous reading
    int16_t analogDiff = tempValue - lastAnalogueValue[analogID];

    //get absolute difference
    if (analogDiff < 0)   analogDiff *= -1;

    if (!(analogDiff >= POTENTIOMETER_CC_STEP)) return;

    uint8_t ccValue = tempValue >> 3;

    //invert CC data if potInverted is true
    if (getAnalogInvertState(analogID))   ccValue = 127 - ccValue;

    uint8_t lowerCClimit = getCClimit(analogID, ccLimitLow);
    uint8_t upperCClimit = getCClimit(analogID, ccLimitHigh);

    //only use map when cc limits are different from defaults
    if ((lowerCClimit != 0) || (upperCClimit != 127))   {

        midi.sendControlChange(getMIDIid(analogID), mapAnalog(ccValue, 0, 127, lowerCClimit, upperCClimit));
        if (sysEx.configurationEnabled()) sysEx.sendID(analogComponent, analogID);

    }

    else {

        midi.sendControlChange(getMIDIid(analogID), ccValue);
        if (sysEx.configurationEnabled()) sysEx.sendID(analogComponent, analogID);

    }

    //update values
    lastAnalogueValue[analogID] = tempValue;

}