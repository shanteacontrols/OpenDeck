#include "LEDs.h"
#include "..\interface\settings\LEDsettings.h"
#include "..\sysex\SysEx.h"
#include "LEDcolors.h"

LEDs::LEDs()    {

    //def const

}

void LEDs::init()   {

    const subtype ledsHardwareParameterSubtype    = { LED_HARDWARE_PARAMETERS, IGNORE_NEW_VALUE, IGNORE_NEW_VALUE };
    const subtype ledsActivationNoteSubtype       = { MAX_NUMBER_OF_LEDS, 0, 127 };
    const subtype ledsStartUpNumberSubtype        = { MAX_NUMBER_OF_LEDS, 0, MAX_NUMBER_OF_LEDS-1 };
    const subtype ledsRGBcolorSubtype             = { MAX_NUMBER_OF_LEDS, 0, MAX_NUMBER_OF_COLORS-1 };
    const subtype ledsStateSubtype                = { MAX_NUMBER_OF_LEDS, 0, LED_STATES, };

    const subtype *ledsSubtypeArray[] = {

        &ledsHardwareParameterSubtype,
        &ledsActivationNoteSubtype,
        &ledsStartUpNumberSubtype,
        &ledsRGBcolorSubtype,
        &ledsStateSubtype

    };

    //define message for sysex configuration
    sysEx.addMessageType(CONF_LED_BLOCK, LED_SUBTYPES);

    //add subtypes
    for (int i=0; i<LED_SUBTYPES; i++)   {

        //define subtype messages
        sysEx.addMessageSubType(CONF_LED_BLOCK, i, ledsSubtypeArray[i]->parameters, ledsSubtypeArray[i]->lowValue, ledsSubtypeArray[i]->highValue);

    }

    board.setLEDblinkTime(getLEDHwParameter(ledsHwParameterBlinkTime));
    board.setLEDTransitionSpeed(getLEDHwParameter(ledsHwParameterFadeTime));

    //run LED animation on start-up
    //startUpRoutine();

}
//
//void LEDs::startUpRoutine() {
//
    ////turn off all LEDs before starting animation
    //allLEDsOff();
//
    //switch (eeprom_read_byte((uint8_t*)EEPROM_LEDS_HW_P_START_UP_ROUTINE))  {
//
        //case 1:
        //oneByOneLED(true, true, true);
        //oneByOneLED(false, false, true);
        //oneByOneLED(true, false, false);
        //oneByOneLED(false, true, true);
        //oneByOneLED(true, false, true);
        //oneByOneLED(false, false, false);
        //break;
//
        //case 2:
        //oneByOneLED(true, false, true);
        //oneByOneLED(false, false, false);
        //break;
//
        //case 3:
        //oneByOneLED(true, true, true);
        //oneByOneLED(false, true, true);
        //break;
//
        //case 4:
        //oneByOneLED(true, false, true);
        //oneByOneLED(true, false, false);
        //break;
//
        //case 5:
        //oneByOneLED(true, false, true);
        //break;
//
        //default:
        //break;
//
    //}
//
    //allLEDsOff();
    //wait(500);
//
//}

//
//void LEDs::oneByOneLED(bool ledDirection, bool singleLED, bool turnOn)  {
//
    ///*
//
    //Function accepts three boolean arguments.
//
    //ledDirection:   true means that LEDs will go from left to right, false from right to left
    //singleLED:      true means that only one LED will be active at the time, false means that LEDs
                    //will turn on one by one until they're all lighted up
//
    //turnOn:         true means that LEDs will be turned on, with all previous LED states being 0
                    //false means that all LEDs are lighted up and they turn off one by one, depending
                    //on second argument
//
    //*/
//
    //uint16_t startUpLEDswitchTime = eeprom_read_byte((uint8_t*)EEPROM_LEDS_HW_P_START_UP_SWITCH_TIME) * 10;
//
    ////while loop counter
    //uint8_t passCounter = 0;
//
    ////index of LED to be processed next
    //uint8_t ledNumber,
            //_ledNumber[MAX_NUMBER_OF_LEDS];
//
    //uint8_t totalNumberOfLEDs = eeprom_read_byte((uint8_t*)EEPROM_LEDS_HW_P_TOTAL_NUMBER);
//
    ////get LED order for start-up routine
    //for (int i=0; i<totalNumberOfLEDs; i++)    _ledNumber[i] = eeprom_read_byte((uint8_t*)EEPROM_LEDS_START_UP_NUMBER_START+i);
//
    ////if second and third argument of function are set to false or
    ////if second argument is set to false and all the LEDs are turned off
    ////light up all LEDs
    //if ((!singleLED && !turnOn) || (checkLEDsOff() && !turnOn)) allLEDsOn();
//
    //if (turnOn) {
//
    ////This part of code deals with situations when previous function call has been
    ////left direction and current one is right and vice versa.
//
    ////On first function call, let's assume the direction was left to right. That would mean
    ////that LEDs had to be processed in this order:
//
    ////LED 1
    ////LED 2
    ////LED 3
    ////LED 4
//
    ////Now, when function is finished, LEDs are not reset yet with allLEDsOff() function to keep
    ////track of their previous states. Next function call is right to left. On first run with
    ////right to left direction, the LED order would be standard LED 4 to LED 1, however, LED 4 has
    ////been already turned on by first function call, so we check if its state is already set, and if
    ////it is we increment or decrement ledNumber by one, depending on previous and current direction.
    ////When function is called second time with direction different than previous one, the number of
    ////times it needs to execute is reduced by one, therefore passCounter is incremented.
//
        ////right-to-left direction
        //if (!ledDirection)  {
//
            ////if last LED is turned on
            //if (board.getLEDstate(_ledNumber[totalNumberOfLEDs-1]))  {
//
                ////LED index is penultimate LED number
                //ledNumber = _ledNumber[totalNumberOfLEDs-2];
                ////increment counter since the loop has to run one cycle less
                //passCounter++;
//
            //}   else    ledNumber = _ledNumber[totalNumberOfLEDs-1]; //led index is last one if last one isn't already on
//
        //}   else //left-to-right direction
//
                ////if first LED is already on
                //if (board.getLEDstate(_ledNumber[0]))    {
//
                ////led index is 1
                //ledNumber = _ledNumber[1];
                ////increment counter
                //passCounter++;
//
                //}   else    ledNumber = _ledNumber[0];
//
    //}   else    {
//
                    ////This is situation when all LEDs are turned on and we're turning them off one by one. Same
                    ////logic applies in both cases (see above). In this case we're not checking for whether the LED
                    ////is already turned on, but whether it's already turned off.
//
                    ////right-to-left direction
                    //if (!ledDirection)  {
//
                        //if (!(board.getLEDstate(_ledNumber[totalNumberOfLEDs-1])))   {
//
                            //ledNumber = _ledNumber[totalNumberOfLEDs-2];
                            //passCounter++;
//
                        //}   else ledNumber = _ledNumber[totalNumberOfLEDs-1];
//
                    //}   else
//
                            //if (!(board.getLEDstate(_ledNumber[0]))) {   //left-to-right direction
//
                                //ledNumber = _ledNumber[1];
                                //passCounter++;
//
                            //}   else ledNumber = _ledNumber[0];
//
        //}
//
    ////on first function call, the while loop is called TOTAL_NUMBER_OF_LEDS+1 times
    ////to get empty cycle after processing last LED
    //while (passCounter < totalNumberOfLEDs+1)   {
//
        //if (passCounter < totalNumberOfLEDs)    {
//
            ////if we're turning LEDs on one by one, turn all the other LEDs off
            //if (singleLED && turnOn)            allLEDsOff();
//
            ////if we're turning LEDs off one by one, turn all the other LEDs on
            //else    if (!turnOn && singleLED)   allLEDsOn();
//
            ////set LED state depending on turnOn parameter
            //if (turnOn) board.setLEDstate(ledNumber, ledOn);
            //else    board.setLEDstate(ledNumber, ledOff);
//
            ////make sure out-of-bound index isn't requested from ledArray
            //if (passCounter < totalNumberOfLEDs-1)  {
//
                ////right-to-left direction
                //if (!ledDirection)  ledNumber = _ledNumber[totalNumberOfLEDs - 2 - passCounter];
//
                ////left-to-right direction
                //else    if (passCounter < totalNumberOfLEDs-1)  ledNumber = _ledNumber[passCounter+1];
//
            //}
//
        //}
//
        ////always increment pass counter
        //passCounter++;
//
        //wait(startUpLEDswitchTime);
//
    //}
//
//}

void LEDs::noteToLEDstate(uint8_t receivedNote, uint8_t receivedVelocity)    {

    uint8_t ledID = getLEDid(receivedNote);

    if (ledID >= 128) return;

    bool newLEDstate;

    //if blinkMode is 1, the LED is blinking
    uint8_t blinkMode = 0;

    if (getLEDHwParameter(ledsHwParameterBlinkTime))    {

        //led blinking is enabled
        if ((receivedVelocity == LED_VELOCITY_C_OFF) || (receivedVelocity == LED_VELOCITY_B_OFF))
        newLEDstate = false;

        else if (

        ((receivedVelocity > LED_VELOCITY_C_OFF) && (receivedVelocity < LED_VELOCITY_B_OFF)) ||
        ((receivedVelocity > LED_VELOCITY_B_OFF) && (receivedVelocity < 128))

        )    newLEDstate = true;

        else return;

        if ((receivedVelocity >= LED_VELOCITY_B_OFF) && (receivedVelocity < 128))
        blinkMode = 1;

    }   else newLEDstate = bool(receivedVelocity);

    board.setLEDstate(ledID, newLEDstate, blinkMode);

}

void LEDs::allLEDsOn()  {

    //turn on all LEDs
    for (int i=0; i<MAX_NUMBER_OF_LEDS; i++)    board.setLEDstate(i, true, false);

}

void LEDs::allLEDsOff() {

    //turn off all LEDs
    for (int i=0; i<(NUMBER_OF_LED_COLUMNS*NUMBER_OF_LED_ROWS); i++)    board.setLEDstate(i, false, false);

}

bool LEDs::checkLEDsOn()    {

    //return true if all LEDs are on
    for (int i=0; i<(NUMBER_OF_LED_COLUMNS*NUMBER_OF_LED_ROWS); i++)    if (board.getLEDstate(i))   return false;
    return true;

}

bool LEDs::checkLEDsOff()   {

    //return true if all LEDs are off
    for (int i=0; i<(NUMBER_OF_LED_COLUMNS*NUMBER_OF_LED_ROWS); i++)    if (!board.getLEDstate(i))   return false;
    return true;

}

uint8_t LEDs::getLEDid(uint8_t midiID)   {

    //match LED activation note with its index
    for (int i=0; i<MAX_NUMBER_OF_LEDS; i++)
        if (getLEDActivationNote(i) == midiID) return i;

    //since 128 is impossible note, return it in case
    //that received note doesn't match any LED
    return 128;

}

bool LEDs::checkLEDstartUpNumber(uint8_t ledID)  {

    //if received start-up number is already assigned to another led, return false
 
    for (int i=0; i<MAX_NUMBER_OF_LEDS; i++)
        if (configuration.readParameter(CONF_LED_BLOCK, ledStartUpNumberSection, i) == ledID)
            return false;

    return true;

}

bool LEDs::checkLEDactivationNote(uint8_t activationNote)    {

    //if received activation note is already assigned to another led, return false

    //led activation note
    for (int i=0; i<MAX_NUMBER_OF_LEDS; i++)
        if (getLEDActivationNote(i) == activationNote)
            return false;

    return true;

}

uint8_t LEDs::getLEDHwParameter(uint8_t parameter)  {

    return configuration.readParameter(CONF_LED_BLOCK, ledHardwareParameterSection, parameter);

}

uint8_t LEDs::getLEDActivationNote(uint8_t ledNumber)   {

    return configuration.readParameter(CONF_LED_BLOCK, ledActivationNoteSection, ledNumber);

}

uint8_t LEDs::getLEDstartUpNumber(uint8_t ledNumber)    {

    return configuration.readParameter(CONF_LED_BLOCK, ledStartUpNumberSection, ledNumber);

}

uint8_t LEDs::getParameter(uint8_t messageType, uint8_t parameterID)   {

    switch(messageType) {

        case ledsHardwareParameterConf:
        return getLEDHwParameter(parameterID);
        break;

        case ledsActivationNoteConf:
        return getLEDActivationNote(parameterID);
        break;

        case ledsStartUpNumberConf:
        return getLEDstartUpNumber(parameterID);
        break;

        case ledsRGBcolorConf:
        break;

        case ledsStateConf:
        break;

    }   return 0;

}


bool LEDs::setLEDHwParameter(uint8_t parameter, uint8_t newParameter) {

    bool returnValue = configuration.writeParameter(CONF_LED_BLOCK, ledHardwareParameterSection, parameter, newParameter);

    if (!returnValue) return false;

    //some special considerations here
    switch((ledsHardwareParameter)parameter)   {

        case ledsHwParameterBlinkTime:
        board.setLEDblinkTime(newParameter);
        break;

        case ledsHwParameterFadeTime:
        board.setLEDTransitionSpeed(newParameter);
        break;

        default:
        break;

    }   return true;

}

bool LEDs::setLEDActivationNote(uint8_t ledNumber, uint8_t ledActNote) {

    if (!checkLEDactivationNote(ledActNote)) return false;

    return configuration.writeParameter(CONF_LED_BLOCK, ledActivationNoteSection, ledNumber, ledActNote);

}

bool LEDs::setLEDstartNumber(uint8_t startNumber, uint8_t ledNumber) {

    return configuration.writeParameter(CONF_LED_BLOCK, ledStartUpNumberSection, startNumber, ledNumber);

}

bool LEDs::setParameter(uint8_t messageType, uint8_t parameter, uint8_t newParameter)   {

    switch(messageType) {

        case ledsHardwareParameterConf:
        return setLEDHwParameter(parameter, newParameter);
        break;

        case ledsActivationNoteConf:
        return setLEDActivationNote(parameter, newParameter);
        break;

        case ledsStartUpNumberConf:
        return setLEDstartNumber(parameter, newParameter);
        break;

        case ledsRGBcolorConf:
        break;

        case ledsStateConf:
        switch ((ledStatesHardwareParameter)newParameter)   {

            case ledStateConstantOff:
            board.setLEDstate(parameter, false, false);
            return true;
            break;

            case ledStateConstantOn:
            board.setLEDstate(parameter, true, false);
            return true;
            break;

            case ledStateBlinkOff:
            board.setLEDstate(parameter, false, true);
            return true;
            break;

            case ledStateBlinkOn:
            board.setLEDstate(parameter, true, true);
            return true;
            break;

            default:
            return false;
            break;

        }
        break;

    }   return false;

}

LEDs leds;