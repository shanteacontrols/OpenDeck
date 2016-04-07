#include "LEDs.h"
#include "../../interface/settings/LEDsettings.h"
#include "../../sysex/SysEx.h"
#include "LEDcolors.h"

LEDs::LEDs()    {

    //def const

}

void LEDs::init()   {

    const subtype ledHardwareParameterSubtype   = { LED_HARDWARE_PARAMETERS, IGNORE_NEW_VALUE, IGNORE_NEW_VALUE };
    const subtype ledActivationNoteSubtype      = { MAX_NUMBER_OF_LEDS, 0, 127 };
    const subtype ledStartUpNumberSubtype       = { MAX_NUMBER_OF_LEDS, 0, MAX_NUMBER_OF_LEDS-1 };
    const subtype ledRGBenabledSubtype          = { MAX_NUMBER_OF_RGB_LEDS, 0, 1 };
    const subtype ledsStateSubtype              = { MAX_NUMBER_OF_LEDS, 0, LED_STATES-1 };

    const subtype *ledsSubtypeArray[] = {

        &ledHardwareParameterSubtype,
        &ledActivationNoteSubtype,
        &ledStartUpNumberSubtype,
        &ledRGBenabledSubtype,
        &ledsStateSubtype

    };

    //define message for sysex configuration
    sysEx.addMessageType(CONF_LED_BLOCK, LED_SUBTYPES);

    //add subtypes
    for (int i=0; i<LED_SUBTYPES; i++)   {

        //define subtype messages
        sysEx.addMessageSubType(CONF_LED_BLOCK, i, ledsSubtypeArray[i]->parameters, ledsSubtypeArray[i]->lowValue, ledsSubtypeArray[i]->highValue);

    }

    board.setLEDblinkTime(getLEDHwParameter(ledHwParameterBlinkTime));
    board.setLEDTransitionSpeed(getLEDHwParameter(ledHwParameterFadeTime));

    //run LED animation on start-up
    startUpAnimation();

}

void LEDs::startUpAnimation() {

    if (!getLEDHwParameter(ledHwParameterTotalLEDnumber) || !getLEDHwParameter(ledHwParameterStartUpSwitchTime))
        return;

    //turn off all LEDs before starting animation
    allLEDsOff();

    switch (getLEDHwParameter(ledHwParameterStartUpRoutine))  {

        case 1:
        oneByOneLED(true, true, true);
        oneByOneLED(false, false, true);
        oneByOneLED(true, false, false);
        oneByOneLED(false, true, true);
        oneByOneLED(true, false, true);
        oneByOneLED(false, false, false);
        break;

        case 2:
        oneByOneLED(true, false, true);
        oneByOneLED(false, false, false);
        break;

        case 3:
        oneByOneLED(true, true, true);
        oneByOneLED(false, true, true);
        break;

        case 4:
        oneByOneLED(true, false, true);
        oneByOneLED(true, false, false);
        break;

        case 5:
        oneByOneLED(true, false, true);
        break;

        default:
        break;

    }

    allLEDsOff();
    wait(1000);

}


void LEDs::oneByOneLED(bool ledDirection, bool singleLED, bool turnOn)  {

    /*

    Function accepts three boolean arguments.

    ledDirection:   true means that LEDs will go from left to right, false from right to left
    singleLED:      true means that only one LED will be active at the time, false means that LEDs
                    will turn on one by one until they're all lighted up

    turnOn:         true means that LEDs will be turned on, with all previous LED states being 0
                    false means that all LEDs are lighted up and they turn off one by one, depending
                    on second argument

    */

    uint16_t startUpLEDswitchTime = getLEDHwParameter(ledHwParameterStartUpSwitchTime) * 10;

    //while loop counter
    uint8_t passCounter = 0;

    //index of LED to be processed next
    uint8_t ledNumber,
            _ledNumber[MAX_NUMBER_OF_LEDS];

    uint8_t totalNumberOfLEDs = getLEDHwParameter(ledHwParameterTotalLEDnumber);

    //get LED order for start-up routine
    for (int i=0; i<totalNumberOfLEDs; i++)
        _ledNumber[i] = getLEDstartUpNumber(i);

    //if second and third argument of function are set to false or
    //if second argument is set to false and all the LEDs are turned off
    //light up all LEDs
    if ((!singleLED && !turnOn) || (checkLEDsOff() && !turnOn)) allLEDsOn();

    if (turnOn) {

    //This part of code deals with situations when previous function call has been
    //left direction and current one is right and vice versa.

    //On first function call, let's assume the direction was left to right. That would mean
    //that LEDs had to be processed in this order:

    //LED 1
    //LED 2
    //LED 3
    //LED 4

    //Now, when function is finished, LEDs are not reset yet with allLEDsOff() function to keep
    //track of their previous states. Next function call is right to left. On first run with
    //right to left direction, the LED order would be standard LED 4 to LED 1, however, LED 4 has
    //been already turned on by first function call, so we check if its state is already set, and if
    //it is we increment or decrement ledNumber by one, depending on previous and current direction.
    //When function is called second time with direction different than previous one, the number of
    //times it needs to execute is reduced by one, therefore passCounter is incremented.

        //right-to-left direction
        if (!ledDirection)  {

            //if last LED is turned on
            if (board.getLEDstate(_ledNumber[totalNumberOfLEDs-1]))  {

                //LED index is penultimate LED number
                ledNumber = _ledNumber[totalNumberOfLEDs-2];
                //increment counter since the loop has to run one cycle less
                passCounter++;

            }   else    ledNumber = _ledNumber[totalNumberOfLEDs-1]; //led index is last one if last one isn't already on

        }   else {

            //left-to-right direction
            //if first LED is already on
            if (board.getLEDstate(_ledNumber[0]))    {

                //led index is 1
                ledNumber = _ledNumber[1];
                //increment counter
                passCounter++;

            }   else ledNumber = _ledNumber[0];

        }

    }   else    {

        //This is situation when all LEDs are turned on and we're turning them off one by one. Same
        //logic applies in both cases (see above). In this case we're not checking for whether the LED
        //is already turned on, but whether it's already turned off.

        //right-to-left direction
        if (!ledDirection)  {

            if (!(board.getLEDstate(_ledNumber[totalNumberOfLEDs-1])))   {

                ledNumber = _ledNumber[totalNumberOfLEDs-2];
                passCounter++;

            }   else ledNumber = _ledNumber[totalNumberOfLEDs-1];

            }   else {

            //left-to-right direction
            if (!(board.getLEDstate(_ledNumber[0]))) {

                ledNumber = _ledNumber[1];
                passCounter++;

            }   else ledNumber = _ledNumber[0];

        }

    }

    //on first function call, the while loop is called TOTAL_NUMBER_OF_LEDS+1 times
    //to get empty cycle after processing last LED
    while (passCounter < totalNumberOfLEDs+1)   {

        if (passCounter < totalNumberOfLEDs)    {

            //if we're turning LEDs on one by one, turn all the other LEDs off
            if (singleLED && turnOn)            allLEDsOff();

            //if we're turning LEDs off one by one, turn all the other LEDs on
            else    if (!turnOn && singleLED)   allLEDsOn();

            //set LED state depending on turnOn parameter
            if (turnOn) board.setLEDstate(ledNumber, colorOnDefault, false);
            else    board.setLEDstate(ledNumber, colorOff, false);

            //make sure out-of-bound index isn't requested from ledArray
            if (passCounter < totalNumberOfLEDs-1)  {

                //right-to-left direction
                if (!ledDirection)  ledNumber = _ledNumber[totalNumberOfLEDs - 2 - passCounter];

                //left-to-right direction
                else    if (passCounter < totalNumberOfLEDs-1)  ledNumber = _ledNumber[passCounter+1];

            }

        }

        //always increment pass counter
        passCounter++;

        wait(startUpLEDswitchTime);

    }

}

ledColor_t LEDs::velocity2color(bool blinkEnabled, uint8_t receivedVelocity) {

    /*

    blinkEnabled:
    constant:
    0-7 off
    8-15 white
    16-23 cyan
    24-31 magenta
    32-39 red
    40-47 blue
    48-55 yellow
    56-63 green

    blink:
    64-71 off
    72-79 white
    80-87 cyan
    88-95 magenta
    96-103 red
    104-111 blue
    112-119 yellow
    120-127 green

    blinkDisabled:
    constant only:
    0-15 off
    16-31 white
    32-47 cyan
    48-63 magenta
    64-79 red
    80-95 blue
    96-111 yellow
    112-127 green

    */

    switch(blinkEnabled) {

        case false:
        return (ledColor_t)(receivedVelocity/16);
        break;

        case true:
        if (receivedVelocity > 63) receivedVelocity -= 64;
        return (ledColor_t)(receivedVelocity/8);
        break;

    }

}

bool LEDs::velocity2blinkState(uint8_t receivedVelocity)    {

    return (receivedVelocity > 63);

}

void LEDs::noteToLEDstate(uint8_t receivedNote, uint8_t receivedVelocity)    {

    bool blinkEnabled_global = getLEDHwParameter(ledHwParameterBlinkTime);
    bool blinkEnabled_led;
    if (!blinkEnabled_global) blinkEnabled_led = false;
    else blinkEnabled_led = velocity2blinkState(receivedVelocity);

    ledColor_t color = velocity2color(blinkEnabled_global, receivedVelocity);

    //match LED activation note with its index
    for (int i=0; i<MAX_NUMBER_OF_LEDS; i++)    {

        if (getLEDActivationNote(i) == receivedNote)  {

            board.setLEDstate(i, color, blinkEnabled_led);

        }

    }

}

void LEDs::allLEDsOn()  {

    //turn on all LEDs
    for (int i=0; i<MAX_NUMBER_OF_LEDS; i++)    board.setLEDstate(i, colorOnDefault, false);

}

void LEDs::allLEDsOff() {

    //turn off all LEDs
    for (int i=0; i<MAX_NUMBER_OF_LEDS; i++)    board.setLEDstate(i, colorOff, false);

}

bool LEDs::checkLEDsOn()    {

    //return true if all LEDs are on
    for (int i=0; i<MAX_NUMBER_OF_LEDS; i++)    if (board.getLEDstate(i))   return false;
    return true;

}

bool LEDs::checkLEDsOff()   {

    //return true if all LEDs are off
    for (int i=0; i<MAX_NUMBER_OF_LEDS; i++)    if (!board.getLEDstate(i))   return false;
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

bool LEDs::getRGBenabled(uint8_t ledNumber) {

    return configuration.readParameter(CONF_LED_BLOCK, ledRGBenabledSection, ledNumber);

}

uint8_t LEDs::getParameter(uint8_t messageType, uint8_t parameterID)   {

    switch(messageType) {

        case ledHardwareParameterConf:
        return getLEDHwParameter(parameterID);
        break;

        case ledActivationNoteConf:
        return getLEDActivationNote(parameterID);
        break;

        case ledStartUpNumberConf:
        return getLEDstartUpNumber(parameterID);
        break;

        case ledRGBenabledConf:
        return getRGBenabled(parameterID);
        break;

        case ledStateConf:
        return (bool)board.getLEDstate(parameterID);
        break;

    }   return 0;

}


bool LEDs::setLEDHwParameter(uint8_t parameter, uint8_t newParameter) {

    //some special considerations here
    switch(parameter)   {

        case ledHwParameterBlinkTime:
        if ((newParameter < BLINK_TIME_MIN) || (newParameter > BLINK_TIME_MAX)) { sysEx.sendError(ERROR_NEW_PARAMETER); return false; }
        break;

        case ledHwParameterFadeTime:
        if ((newParameter < FADE_TIME_MIN) || (newParameter > FADE_TIME_MAX)) { sysEx.sendError(ERROR_NEW_PARAMETER); return false; }
        break;

        case ledHwParameterStartUpSwitchTime:
        if ((newParameter < START_UP_SWITCH_TIME_MIN) || (newParameter > START_UP_SWITCH_TIME_MAX)) { sysEx.sendError(ERROR_NEW_PARAMETER); return false; }
        break;

        case ledHwParameterStartUpRoutine:
        if (newParameter > NUMBER_OF_START_UP_ANIMATIONS) { sysEx.sendError(ERROR_NEW_PARAMETER); return false; }
        break;

        default:
        break;

    }

    bool returnValue = configuration.writeParameter(CONF_LED_BLOCK, ledHardwareParameterSection, parameter, newParameter);

    if (returnValue)    {

        switch(newParameter)    {

            case ledHwParameterBlinkTime:
            board.setLEDblinkTime(newParameter);
            break;

            case ledHwParameterFadeTime:
            board.setLEDTransitionSpeed(newParameter);
            break;

        }   return true;

    }   return false;

}

bool LEDs::setLEDActivationNote(uint8_t ledNumber, uint8_t ledActNote) {

    return configuration.writeParameter(CONF_LED_BLOCK, ledActivationNoteSection, ledNumber, ledActNote);

}

bool LEDs::setLEDstartNumber(uint8_t startNumber, uint8_t ledNumber) {

    return configuration.writeParameter(CONF_LED_BLOCK, ledStartUpNumberSection, startNumber, ledNumber);

}

bool LEDs::setRGBenabled(uint8_t ledNumber, bool state) {

    return configuration.writeParameter(CONF_LED_BLOCK, ledRGBenabledSection, ledNumber, state);

}

bool LEDs::setParameter(uint8_t messageType, uint8_t parameter, uint8_t newParameter)   {

    switch(messageType) {

        case ledHardwareParameterConf:
        return setLEDHwParameter(parameter, newParameter);
        break;

        case ledActivationNoteConf:
        return setLEDActivationNote(parameter, newParameter);
        break;

        case ledStartUpNumberConf:
        return setLEDstartNumber(parameter, newParameter);
        break;

        case ledRGBenabledConf:
        return setRGBenabled(parameter, newParameter);
        break;

        case ledStateConf:
        switch ((ledStatesHardwareParameter)newParameter)   {

            case ledStateOff:
            board.setLEDstate(parameter, colorOff, false);
            return true;
            break;

            case ledStateConstantWhite:
            board.setLEDstate(parameter, colorWhite, false);
            return true;
            break;

            case ledStateConstantCyan:
            board.setLEDstate(parameter, colorCyan, false);
            return true;
            break;

            case ledStateConstantMagenta:
            board.setLEDstate(parameter, colorMagenta, false);
            return true;
            break;

            case ledStateConstantRed:
            board.setLEDstate(parameter, colorRed, false);
            return true;
            break;

            case ledStateConstantBlue:
            board.setLEDstate(parameter, colorBlue, false);
            return true;
            break;

            case ledStateConstantYellow:
            board.setLEDstate(parameter, colorYellow, false);
            return true;
            break;

            case ledStateConstantGreen:
            board.setLEDstate(parameter, colorGreen, false);
            return true;
            break;

            case ledStateBlinkWhite:
            board.setLEDstate(parameter, colorWhite, true);
            return true;
            break;

            case ledStateBlinkCyan:
            board.setLEDstate(parameter, colorCyan, true);
            return true;
            break;

            case ledStateBlinkMagenta:
            board.setLEDstate(parameter, colorMagenta, true);
            return true;
            break;

            case ledStateBlinkRed:
            board.setLEDstate(parameter, colorRed, true);
            return true;
            break;

            case ledStateBlinkBlue:
            board.setLEDstate(parameter, colorBlue, true);
            return true;
            break;

            case ledStateBlinkYellow:
            board.setLEDstate(parameter, colorYellow, true);
            return true;
            break;

            case ledStateBlinkGreen:
            board.setLEDstate(parameter, colorGreen, true);
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
