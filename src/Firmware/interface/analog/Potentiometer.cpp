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
    if (configuration.readParameter(CONF_BLOCK_ANALOG, analogInvertedSection, analogID))   ccValue = 127 - ccValue;

    uint8_t lowerCClimit = configuration.readParameter(CONF_BLOCK_ANALOG, analogCClowerLimitSection, analogID);
    uint8_t upperCClimit = configuration.readParameter(CONF_BLOCK_ANALOG, analogCCupperLimitSection, analogID);

    //only use map when cc limits are different from defaults
    if ((lowerCClimit != 0) || (upperCClimit != 127))   {

        midi.sendControlChange(configuration.readParameter(CONF_BLOCK_ANALOG, analogMIDIidSection, analogID), mapAnalog_uint8(ccValue, 0, 127, lowerCClimit, upperCClimit));
        //if (sysEx.configurationEnabled())
            //sysEx.sendComponentID(CONF_BLOCK_ANALOG, analogID);

    }

    else {

        midi.sendControlChange(configuration.readParameter(CONF_BLOCK_ANALOG, analogMIDIidSection, analogID), ccValue);
        //if (sysEx.configurationEnabled())
            //sysEx.sendComponentID(CONF_BLOCK_ANALOG, analogID);

    }

    //update values
    lastAnalogueValue[analogID] = tempValue;

}
