#include "OpenDeck.h"

//potentiometer must exceed this value before sending new value
#define POTENTIOMETER_CC_STEP 8

void OpenDeck::checkPotentiometerValue(int16_t tempValue, uint8_t analogID)  {

    //calculate difference between current and previous reading
    int16_t analogDiff = tempValue - lastAnalogueValue[analogID];

    //get absolute difference
    if (analogDiff < 0)   analogDiff *= -1;

    if (!(analogDiff >= POTENTIOMETER_CC_STEP)) return;

    uint8_t ccValue = tempValue >> 3;

    //invert CC data if potInverted is true
    if (getAnalogInvertState(analogID))   ccValue = 127 - ccValue;

    //only use map when cc limits are different from defaults
    if ((analogLowerLimit[analogID] != 0) || (analogUpperLimit[analogID] != 127))   {

        sendControlChange(ccNumber[analogID], mapAnalog(ccValue, 0, 127, analogLowerLimit[analogID], analogUpperLimit[analogID]), _CCchannel);
        if (sysExEnabled) sysExSendID(analog, analogID);

    }

    else {

        sendControlChange(ccNumber[analogID], ccValue, _CCchannel);
        if (sysExEnabled) sysExSendID(analog, analogID);

    }

    //update values
    lastAnalogueValue[analogID] = tempValue;

}