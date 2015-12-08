#include "LEDs.h"
#include "sysex/ProtocolDefinitions.h"
#include "eeprom/EEPROMsettings.h"
#include "LEDsettings.h"
#include "sysex/SysEx.h"
#include "BitManipulation.h"

typedef enum {

    ledsHardwareParameterConf,
    ledsActivationNoteConf,
    ledsStartUpNumberConf,
    ledsStateConf,
    LEDS_SUBTYPES

} sysExMessageSubTypeLEDs;

typedef enum {

    ledsHwParameterTotalLEDnumber,
    ledsHwParameterBlinkTime,
    ledsHwParameterStartUpSwitchTime,
    ledsHwParameterStartUpRoutine,
    ledsHwParameterFadeTime,
    LEDS_HARDWARE_PARAMETERS

} ledsHardwareParameter;

typedef enum {

    ledStateConstantOff,
    ledStateConstantOn,
    ledStateBlinkOff,
    ledStateBlinkOn,
    LED_STATES

} ledStatesHardwareParameter;

subtype ledsHardwareParameterSubtype    = { LEDS_HARDWARE_PARAMETERS, IGNORE_NEW_VALUE, IGNORE_NEW_VALUE };
subtype ledsActivationNoteSubtype       = { MAX_NUMBER_OF_LEDS, 0, 127 };
subtype ledsStartUpNumberSubtype        = { MAX_NUMBER_OF_LEDS, 0, MAX_NUMBER_OF_LEDS-1 };
subtype ledsStateSubtype                = { MAX_NUMBER_OF_LEDS, 0, LED_STATES };

const uint8_t ledsSubtypeArray[] = {

    ledsHardwareParameterConf,
    ledsActivationNoteConf,
    ledsStartUpNumberConf,
    ledsStateConf

};

const uint8_t ledsParametersArray[] = {

    ledsHardwareParameterSubtype.parameters,
    ledsActivationNoteSubtype.parameters,
    ledsStartUpNumberSubtype.parameters,
    ledsStateSubtype.parameters

};

const uint8_t ledsNewParameterLowArray[] = {

    ledsHardwareParameterSubtype.lowValue,
    ledsActivationNoteSubtype.lowValue,
    ledsStartUpNumberSubtype.lowValue,
    ledsStateSubtype.lowValue

};

const uint8_t ledsNewParameterHighArray[] = {

    ledsHardwareParameterSubtype.highValue,
    ledsActivationNoteSubtype.highValue,
    ledsStartUpNumberSubtype.highValue,
    ledsStateSubtype.highValue

};

LEDs::LEDs()    {

    //def const

}

void LEDs::init()   {

    getConfiguration();

    //define message for sysex configuration
    sysEx.addMessageType(SYS_EX_MT_LEDS, LEDS_SUBTYPES);

    //add subtypes
    for (int i=0; i<LEDS_SUBTYPES; i++)   {

        //define subtype messages
        sysEx.addMessageSubType(SYS_EX_MT_LEDS, ledsSubtypeArray[i], ledsParametersArray[i], ledsNewParameterLowArray[i], ledsNewParameterHighArray[i]);

    }

    //run LED animation on start-up
    //startUpRoutine();

}

void LEDs::getConfiguration()   {

    board.setLEDblinkTime(eeprom_read_byte((uint8_t*)EEPROM_LEDS_HW_P_BLINK_TIME)*100);

}

void LEDs::startUpRoutine() {

    //turn off all LEDs before starting animation
    allLEDsOff();

    switch (eeprom_read_byte((uint8_t*)EEPROM_LEDS_HW_P_START_UP_ROUTINE))  {

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
    wait(500);

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

    uint16_t startUpLEDswitchTime = eeprom_read_byte((uint8_t*)EEPROM_LEDS_HW_P_START_UP_SWITCH_TIME) * 10;

    //while loop counter
    uint8_t passCounter = 0;

    //index of LED to be processed next
    uint8_t ledNumber,
            _ledNumber[MAX_NUMBER_OF_LEDS];

    uint8_t totalNumberOfLEDs = eeprom_read_byte((uint8_t*)EEPROM_LEDS_HW_P_TOTAL_NUMBER);

    //get LED order for start-up routine
    for (int i=0; i<totalNumberOfLEDs; i++)    _ledNumber[i] = eeprom_read_byte((uint8_t*)EEPROM_LEDS_START_UP_NUMBER_START+i);

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

        }   else //left-to-right direction

                //if first LED is already on
                if (board.getLEDstate(_ledNumber[0]))    {

                //led index is 1
                ledNumber = _ledNumber[1];
                //increment counter
                passCounter++;

                }   else    ledNumber = _ledNumber[0];

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

                    }   else

                            if (!(board.getLEDstate(_ledNumber[0]))) {   //left-to-right direction

                                ledNumber = _ledNumber[1];
                                passCounter++;

                            }   else ledNumber = _ledNumber[0];

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
            if (turnOn) board.setLEDstate(ledNumber, ledOn);
            else    board.setLEDstate(ledNumber, ledOff);

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

void LEDs::noteToLEDstate(uint8_t receivedNote, uint8_t receivedVelocity)    {

    bool newLEDstate;

    //if blinkMode is 1, the LED is blinking
    uint8_t blinkMode = 0;

    if ((receivedVelocity == LED_VELOCITY_C_OFF) || (receivedVelocity == LED_VELOCITY_B_OFF))
    newLEDstate = false;

    else if (

    ((receivedVelocity > LED_VELOCITY_C_OFF) && (receivedVelocity < LED_VELOCITY_B_OFF)) ||
    ((receivedVelocity > LED_VELOCITY_B_OFF) && (receivedVelocity < 128))

    )    newLEDstate = true;

    else return;

    if ((receivedVelocity >= LED_VELOCITY_B_OFF) && (receivedVelocity < 128))
    blinkMode = 1;

    uint8_t ledID = getLEDid(receivedNote);
    if (ledID & MAX_MIDI_VALUE_MASK)
        handleLED(newLEDstate, blinkMode, ledID);

}

void LEDs::allLEDsOn()  {

    //turn on all LEDs
    for (int i=0; i<MAX_NUMBER_OF_LEDS; i++)    board.setLEDstate(i, ledOn);

}

void LEDs::allLEDsOff() {

    //turn off all LEDs
    for (int i=0; i<(NUMBER_OF_LED_COLUMNS*NUMBER_OF_LED_ROWS); i++)    board.setLEDstate(i, ledOff);

}

void LEDs::allLEDsBlink()   {

    //turn on all LEDs
    for (int i=0; i<MAX_NUMBER_OF_LEDS; i++)    board.setLEDstate(i, ledBlinkOff);
    board.ledBlinkingStart();

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

uint8_t LEDs::getLEDActivationNote(uint8_t ledNumber)   {

    return eeprom_read_byte((uint8_t*)EEPROM_LEDS_ACT_NOTE_START+ledNumber);

}

bool LEDs::checkLEDstartUpNumber(uint8_t ledID)  {

    for (int i=0; i<MAX_NUMBER_OF_LEDS; i++)
        if (eeprom_read_byte((uint8_t*)EEPROM_LEDS_START_UP_NUMBER_START+i) == ledID)
            return false;

    return true;

}

bool LEDs::checkLEDactivationNote(uint8_t ledID)    {

    //led activation note
    for (int i=0; i<MAX_NUMBER_OF_LEDS; i++)
        if (getLEDActivationNote(i) == ledID)
            return false;

    return true;

}

void LEDs::handleLED(bool newLEDstate, bool blinkMode, uint8_t ledNumber) {

    /*

    LED state is stored into one byte (ledState). The bits have following meaning (7 being the MSB bit):

    7: x
    6: x
    5: x
    4: Blink bit (timer changes this bit)
    3: "Remember" bit, used to restore previous LED state
    2: LED is active (either it blinks or it's constantly on), this bit is OR function between bit 0 and 1
    1: LED blinks
    0: LED is constantly turned on

    */

    //if blink note is received, and blinking is disabled, exit the function
    //if (blinkMode && (!(bitRead(ledFeatures, SYS_EX_FEATURES_LEDS_BLINK))))
        //return;

    uint8_t state = board.getLEDstate(ledNumber);

    switch (newLEDstate) {

        case false:
        //note off event

        //if remember bit is set
        if (bitRead(state, LED_REMEMBER_BIT))   {

            //if note off for blink state is received
            //clear remember bit and blink bits
            //set constant state bit
            if (blinkMode) state = ledOn;
            //else clear constant state bit and remember bit
            //set blink bits
            else           state = ledBlinkOn;

        }   else state = ledOff;

        break;

        case true:
        //note on event

        if ((!blinkMode) && bitRead(state, LED_BLINK_ON_BIT))   state = ledOnRemember;
        else if ((blinkMode) && bitRead(state, LED_ON_BIT))     state = ledBlinkRemember;

        else    {

            bitWrite(state, LED_ACTIVE_BIT, 1);
            if (blinkMode)  {

                bitWrite(state, LED_BLINK_ON_BIT, 1);
                bitWrite(state, LED_BLINK_STATE_BIT, 1);

            }   else bitWrite(state, LED_ON_BIT, 1);

        }

    }

    board.setLEDstate(ledNumber, state);
    if (blinkMode && newLEDstate)   board.ledBlinkingStart();
    else    checkBlinkLEDs();

}

void LEDs::checkBlinkLEDs() {

    //this function will disable blinking
    //if none of the LEDs is in blinking state

    //else it will enable it

    bool _blinkEnabled = false;
    uint8_t ledState;

    //if any LED is blinking, set timerState to true and exit the loop
    for (int i=0; i<MAX_NUMBER_OF_LEDS; i++)    {

        ledState = board.getLEDstate(i);

        if (bitRead(ledState, LED_BLINK_ON_BIT)) {

            _blinkEnabled = true;
            break;

        }

    }

    if (_blinkEnabled)  board.ledBlinkingStart();

    //don't bother reseting variables if blinking is already disabled
    else    if (!_blinkEnabled && board.ledBlinkingActive()) {

        //reset blinkState to default value
        board.ledBlinkingStop();

    }

}

bool LEDs::setLEDHwParameter(uint8_t parameter, uint8_t newParameter) {

    uint16_t address;

    switch((ledsHardwareParameter)parameter)   {

        case ledsHwParameterTotalLEDnumber:
        //set total number of LEDs (needed for start-up routine)
        address = EEPROM_LEDS_HW_P_TOTAL_NUMBER;
        if (newParameter == RESET_VALUE) newParameter = pgm_read_byte(&(defConf[address]));
        return (eeprom_read_byte((uint8_t*)address) == newParameter);
        break;

        case ledsHwParameterBlinkTime:
        //blink time
        address = EEPROM_LEDS_HW_P_BLINK_TIME;
        if (newParameter == RESET_VALUE) newParameter = pgm_read_byte(&(defConf[address]));
        board.resetLEDblinkCounter();
        board.setLEDblinkTime(newParameter*100);
        eeprom_update_byte((uint8_t*)address, newParameter);
        return (eeprom_read_byte((uint8_t*)address) == newParameter);
        break;

        case ledsHwParameterStartUpSwitchTime:
        //start-up led switch time
        address = EEPROM_LEDS_HW_P_START_UP_SWITCH_TIME;
        if (newParameter == RESET_VALUE) newParameter = pgm_read_byte(&(defConf[address]));
        eeprom_update_byte((uint8_t*)address, newParameter);
        return (eeprom_read_byte((uint8_t*)address) == newParameter);
        break;

        case ledsHwParameterStartUpRoutine:
        //set start-up routine pattern
        address = EEPROM_LEDS_HW_P_START_UP_ROUTINE;
        if (newParameter == RESET_VALUE) newParameter = pgm_read_byte(&(defConf[address]));
        eeprom_update_byte((uint8_t*)address, newParameter);
        return (eeprom_read_byte((uint8_t*)address) == newParameter);
        break;

        case ledsHwParameterFadeTime:
        //pwm fade speed
        address = EEPROM_LEDS_HW_P_FADE_SPEED;
        if (newParameter == RESET_VALUE) newParameter = pgm_read_byte(&(defConf[address]));
        board.resetLEDtransitions();
        eeprom_update_byte((uint8_t*)address, newParameter);
        if (eeprom_read_byte((uint8_t*)address) == newParameter) {

            board.setLEDTransitionSpeed(newParameter);
            return true;

        } return false;
        break;

        default:
        break;

    }   return false;

}

uint8_t LEDs::getLEDHwParameter(uint8_t parameter)  {

    switch (parameter)  {

        case ledsHwParameterTotalLEDnumber:
        return eeprom_read_byte((uint8_t*)EEPROM_LEDS_HW_P_TOTAL_NUMBER);
        break;

        case ledsHwParameterBlinkTime:
        return eeprom_read_byte((uint8_t*)EEPROM_LEDS_HW_P_BLINK_TIME);
        break;

        case ledsHwParameterStartUpSwitchTime:
        return eeprom_read_byte((uint8_t*)EEPROM_LEDS_HW_P_START_UP_SWITCH_TIME);
        break;

        case ledsHwParameterStartUpRoutine:
        return eeprom_read_byte((uint8_t*)EEPROM_LEDS_HW_P_START_UP_ROUTINE);
        break;

        case ledsHwParameterFadeTime:
        return eeprom_read_byte((uint8_t*)EEPROM_LEDS_HW_P_FADE_SPEED);
        break;

        default:
        break;

    }   return INVALID_VALUE;

}

uint8_t LEDs::getParameter(uint8_t messageType, uint8_t parameter)   {

    switch(messageType) {

        case ledsHardwareParameterConf:
        return getLEDHwParameter(parameter);
        break;

        case ledsActivationNoteConf:
        return getLEDActivationNote(parameter);
        break;

        case ledsStartUpNumberConf:
        return eeprom_read_byte((uint8_t*)EEPROM_LEDS_START_UP_NUMBER_START+parameter);
        break;

        case ledsStateConf:
        return board.getLEDstate(parameter);
        break;

        default:
        break;

    }   return INVALID_VALUE;

}

bool LEDs::setParameter(uint8_t messageType, uint8_t parameter, uint8_t newParameter)   {

    switch((sysExMessageSubTypeLEDs)messageType) {

        case ledsHardwareParameterConf:
        return setLEDHwParameter(parameter, newParameter);
        break;

        case ledsActivationNoteConf:
        return setLEDActivationNote(parameter, newParameter);
        break;

        case ledsStartUpNumberConf:
        return setLEDstartNumber(parameter, newParameter);
        break;

        case ledsStateConf:
        switch ((ledStatesHardwareParameter)newParameter)   {

            case ledStateConstantOff:
            handleLED(false, false, parameter);
            return true;
            break;

            case ledStateConstantOn:
            handleLED(true, false, parameter);
            return true;
            break;

            case ledStateBlinkOff:
            handleLED(false, true, parameter);
            return true;
            break;

            case ledStateBlinkOn:
            handleLED(true, true, parameter);
            return true;
            break;

            default:
            return false;
            break;

        }
        break;

        default:
        break;

    }   return false;

}

bool LEDs::setLEDActivationNote(uint8_t ledNumber, uint8_t ledActNote) {

    uint16_t address = EEPROM_LEDS_ACT_NOTE_START+ledNumber;

    if (ledActNote == RESET_VALUE) ledActNote = pgm_read_byte(&(defConf[address]));

    eeprom_update_byte((uint8_t*)address, ledActNote);

    return (ledActNote == eeprom_read_byte((uint8_t*)address));

}

bool LEDs::setLEDstartNumber(uint8_t startNumber, uint8_t ledNumber) {

    uint16_t address = EEPROM_LEDS_START_UP_NUMBER_START+startNumber;

    if (ledNumber == RESET_VALUE) ledNumber = pgm_read_byte(&(defConf[address]));

    eeprom_update_byte((uint8_t*)address, ledNumber);

    return ledNumber == eeprom_read_byte((uint8_t*)address);

}

LEDs leds;