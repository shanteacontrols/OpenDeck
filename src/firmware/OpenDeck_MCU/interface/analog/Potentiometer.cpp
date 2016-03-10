#include "Analog.h"
#include "../../sysex/SysEx.h"
#include "../../eeprom/Blocks.h"

//potentiometer must exceed this value before sending new value
#define POTENTIOMETER_CC_STEP 8

void Analog::checkPotentiometerValue(uint8_t analogID, int16_t tempValue)  {

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

        midi.sendControlChange(getMIDIid(analogID), mapAnalog_uint8(ccValue, 0, 127, lowerCClimit, upperCClimit));
        if (sysEx.configurationEnabled()) sysEx.sendComponentID(CONF_ANALOG_BLOCK, analogID);

    }

    else {

        midi.sendControlChange(getMIDIid(analogID), ccValue);
        if (sysEx.configurationEnabled()) sysEx.sendComponentID(CONF_ANALOG_BLOCK, analogID);

    }

    //update values
    lastAnalogueValue[analogID] = tempValue;

}
