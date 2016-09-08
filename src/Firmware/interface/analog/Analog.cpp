#include "Analog.h"
#include "../../sysex/SysEx.h"

#define DIGITAL_VALUE_THRESHOLD 0x3E8

Analog::Analog()    {

    //def const
    analogDebounceCounter = 0;

}

void Analog::update()   {

    //if (!core.analogDataAvailable()) return;
//
    //addAnalogSamples();
    //int16_t analogData;
//
    ////check values
    //for (int i=0; i<MAX_NUMBER_OF_ANALOG; i++)    {
//
        ////don't process component if it's not enabled
        //if (!configuration.readParameter(CONF_BLOCK_ANALOG, analogEnabledSection, i)) continue;
//
        //if (!configuration.readParameter(CONF_BLOCK_ANALOG, analogDigitalEnabledSection, i))    {
//
            ////if (!analogValuesSampled()) continue;  //three samples are needed for analog values
//
            ////get median value from three analog samples for better accuracy
            //analogData = getMedianValue(i);
//
            //analogType_t type = (analogType_t)configuration.readParameter(CONF_BLOCK_ANALOG, analogTypeSection, i);
//
            //switch(type) {
//
                //case potentiometer:
                //checkPotentiometerValue(i, analogData);
                //break;
//
                //case fsr:
                //checkFSRvalue(i, analogData);
                //break;
//
                //case ldr:
                //break;
//
                //default:
                //break;
//
            //}
////
        //}   else {
//
            ////analogData = core.getAnalogValue(i);
            ////bool state = analogData > DIGITAL_VALUE_THRESHOLD;
            ////Buttons::processButton(i+MAX_NUMBER_OF_BUTTONS, state);
//
        //}
//
    //}

    if (!core.analogDataAvailable()) return;

    addAnalogSamples();
    //three samples are needed
    bool sampled = analogValuesSampled();

    int16_t analogData;

    //check values
    for (int i=0; i<MAX_NUMBER_OF_ANALOG; i++)    {

        //don't process component if it's not enabled
        if (!configuration.readParameter(CONF_BLOCK_ANALOG, analogEnabledSection, i)) continue;

        if (!configuration.readParameter(CONF_BLOCK_ANALOG, analogDigitalEnabledSection, i))    {

            //three samples are needed
            if (!sampled) continue;

            //get median value from three analog samples for better accuracy
            analogData = getMedianValue(i);
            analogType_t type = (analogType_t)configuration.readParameter(CONF_BLOCK_ANALOG, analogTypeSection, i);

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

        }   else {

            analogData = core.getAnalogValue(i);
            bool state = analogData > DIGITAL_VALUE_THRESHOLD;
            Buttons::processButton(i+MAX_NUMBER_OF_BUTTONS, state, false);

        }

    }

}

void Analog::addAnalogSamples() {

    for (int i=0; i<MAX_NUMBER_OF_ANALOG; i++)
        analogSample[i][analogDebounceCounter] = core.getAnalogValue(i); //get raw analog reading

    analogDebounceCounter++;

}

bool Analog::analogValuesSampled() {

    if (analogDebounceCounter == NUMBER_OF_SAMPLES) {

        analogDebounceCounter = 0;
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

void Analog::debounceReset(uint16_t index)    {

    lastAnalogueValue[index] = 0;
    fsrLastAfterTouchValue[index] = 0;

    uint8_t arrayIndex = index/8;
    uint8_t fsrIndex = index - 8*arrayIndex;

    bitWrite(fsrPressed[arrayIndex], fsrIndex, false);

}

Analog analog;
