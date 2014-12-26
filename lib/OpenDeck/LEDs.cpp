/*

OpenDECK library v1.3
File: LEDs.cpp
Last revision date: 2014-12-25
Author: Igor Petrovic

*/

#include "OpenDeck.h"
#include <avr/eeprom.h>
#include "Ownduino.h"


void OpenDeck::startUpRoutine() {

    //turn off all LEDs before starting animation
    allLEDsOff();

    switch (eeprom_read_byte((uint8_t*)EEPROM_LED_HW_P_START_UP_ROUTINE))  {

        case 1:
        openDeck.oneByOneLED(true, true, true);
        openDeck.oneByOneLED(false, false, true);
        openDeck.oneByOneLED(true, false, false);
        openDeck.oneByOneLED(false, true, true);
        openDeck.oneByOneLED(true, false, true);
        openDeck.oneByOneLED(false, false, false);
        openDeck.allLEDsOff();
        break;

        case 2:
        openDeck.oneByOneLED(true, false, true);
        openDeck.oneByOneLED(false, false, false);
        openDeck.allLEDsOff();
        break;

        case 3:
        openDeck.oneByOneLED(true, true, true);
        openDeck.oneByOneLED(false, true, true);
        openDeck.allLEDsOff();
        break;

        case 4:
        openDeck.oneByOneLED(true, false, true);
        openDeck.oneByOneLED(true, false, false);
        openDeck.allLEDsOff();
        break;

        default:
        break;

    }

}

void OpenDeck::oneByOneLED(bool ledDirection, bool singleLED, bool turnOn)  {

    /*

    Function accepts three boolean arguments.

    ledDirection:   true means that LEDs will go from left to right, false from right to left
    singleLED:      true means that only one LED will be active at the time, false means that LEDs
                    will turn on one by one until they're all lighted up

    turnOn:         true means that LEDs will be turned on, with all previous LED states being 0
                    false means that all LEDs are lighted up and they turn off one by one, depending
                    on second argument

    */

    uint16_t _startUpLEDswitchTime = eeprom_read_byte((uint8_t*)EEPROM_LED_HW_P_START_UP_SWITCH_TIME) * 10;

    //remember previously active column
    static int8_t previousColumn = -1;

    //while loop counter
    uint8_t passCounter = 0;

    //reset the timer on each function call
    uint32_t startUpTimer = 0;

    //index of LED to be processed next
    uint8_t ledNumber,
            _ledNumber[MAX_NUMBER_OF_LEDS];

    //get LED order for start-up routine
    for (int i=0; i<totalNumberOfLEDs; i++)    _ledNumber[i] = eeprom_read_byte((uint8_t*)EEPROM_LED_START_UP_NUMBER_START+i);

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
            if (ledOn(_ledNumber[totalNumberOfLEDs-1]))  {

                //LED index is penultimate LED number
                ledNumber = _ledNumber[totalNumberOfLEDs-2];
                //increment counter since the loop has to run one cycle less
                passCounter++;

            }   else    ledNumber = _ledNumber[totalNumberOfLEDs-1]; //led index is last one if last one isn't already on
            
        }   else //left-to-right direction

                //if first LED is already on
                if (ledOn(_ledNumber[0]))    {

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
                        
                        if (!(ledOn(_ledNumber[totalNumberOfLEDs-1])))   {

                            ledNumber = _ledNumber[totalNumberOfLEDs-2];
                            passCounter++;

                        }   else ledNumber = _ledNumber[totalNumberOfLEDs-1];

                    }   else

                            if (!(ledOn(_ledNumber[0]))) {   //left-to-right direction

                                ledNumber = _ledNumber[1];
                                passCounter++;

                            }   else ledNumber = _ledNumber[0];

        }

    //on first function call, the while loop is called TOTAL_NUMBER_OF_LEDS+1 times
    //to get empty cycle after processing last LED
    while (passCounter < totalNumberOfLEDs+1)   {

        uint8_t activeColumn = getActiveColumn();

        if (previousColumn != activeColumn)   {

            //only process LED after defined time
            if ((millis() - startUpTimer) > _startUpLEDswitchTime)  {

                if (passCounter < totalNumberOfLEDs)    {

                    //if we're turning LEDs on one by one, turn all the other LEDs off
                    if (singleLED && turnOn)            allLEDsOff();

                    //if we're turning LEDs off one by one, turn all the other LEDs on
                    else    if (!turnOn && singleLED)   allLEDsOn();

                    //set LED state depending on turnOn parameter
                    if (turnOn) turnOnLED(ledNumber);
                        else    turnOffLED(ledNumber);

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

            //update timer
            startUpTimer = millis();

        }

            //check if there is any LED to be turned on
            checkLEDs(activeColumn);

            //update last column with current
            previousColumn = activeColumn;

        }

    }

}

void OpenDeck::allLEDsOn()  {

    //turn on all LEDs
    for (int i=0; i<MAX_NUMBER_OF_LEDS; i++)    setConstantLEDstate(i);

}

void OpenDeck::allLEDsOff() {

    //turn off all LEDs
    for (int i=0; i<MAX_NUMBER_OF_LEDS; i++)    ledState[i] = 0x00;

}

void OpenDeck::turnOnLED(uint8_t ledNumber) {

    setConstantLEDstate(ledNumber);

}

void OpenDeck::turnOffLED(uint8_t ledNumber)    {

    ledState[ledNumber] = 0x00;

}

void OpenDeck::storeReceivedNoteOn(uint8_t channel, uint8_t note, uint8_t velocity)  {

    receivedChannel = channel;
    receivedNote = note;
    receivedVelocity = velocity;

    receivedNoteOnProcessed = false;

}

void OpenDeck::checkReceivedNoteOn()  {

    if (!receivedNoteOnProcessed) setLEDState();

}

void OpenDeck::checkLEDs(uint8_t currentColumn)  {

    if ((_board != 0) && (bitRead(hardwareEnabled, SYS_EX_HW_CONFIG_LEDS)))    {

        if ((blinkEnabled) && (bitRead(ledFeatures, SYS_EX_FEATURES_LEDS_BLINK)))   switchBlinkState();

        //if there is an active LED in current column, turn on LED row
        for (int i=0; i<_numberOfLEDrows; i++)  {

            if (ledOn(currentColumn+i*_numberOfColumns))
                ledRowOn(i);

        }

    }

}

bool OpenDeck::ledOn(uint8_t ledNumber) {

    if  (

        ledState[ledNumber] == 0x05 ||
        ledState[ledNumber] == 0x15 ||
        ledState[ledNumber] == 0x16 ||
        ledState[ledNumber] == 0x1D ||
        ledState[ledNumber] == 0x0D ||
        ledState[ledNumber] == 0x17

    )   return true;

    return false;

}

bool OpenDeck::checkLEDsOn()    {

    //return true if all LEDs are on
    for (int i=0; i<MAX_NUMBER_OF_LEDS; i++)    if (ledState[i] != 0)   return false;
    return true;

}

bool OpenDeck::checkLEDsOff()   {

    //return true if all LEDs are off
    for (int i=0; i<MAX_NUMBER_OF_LEDS; i++)    if (ledState[i] == 0)   return false;
    return true;

}

void OpenDeck::checkBlinkLEDs() {

    //this function will disable blinking
    //if none of the LEDs is in blinking state

    //else it will enable it

    bool _blinkEnabled = false;

    //if any LED is blinking, set timerState to true and exit the loop
    for (int i=0; i<MAX_NUMBER_OF_LEDS; i++)
    
    if (checkBlinkState(i)) {

        _blinkEnabled = true;
        break;

    }

    if (_blinkEnabled)  blinkEnabled = true;

    //don't bother reseting variables if blinking is already disabled
    else    if (!_blinkEnabled && blinkEnabled) {

                //reset blinkState to default value
                blinkState = true;
                blinkTimerCounter = 0;
                blinkEnabled = false;

            }

}

bool OpenDeck::checkBlinkState(uint8_t ledNumber)   {

    //function returns true if blinking bit in ledState is set
    return ((ledState[ledNumber] >> 1) & (0x01));

}

void OpenDeck::handleLED(bool currentLEDstate, bool blinkMode, uint8_t _ledNumber) {

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
    if (blinkMode && (!(bitRead(ledFeatures, SYS_EX_FEATURES_LEDS_BLINK))))
        return;

    uint8_t ledNumber = _ledNumber;

    if (ledNumber < 128)    {

        switch (currentLEDstate) {

            case false:
            //note off event

            //if remember bit is set
            if ((ledState[ledNumber] >> 3) & (0x01))   {

                //if note off for blink state is received
                //clear remember bit and blink bits
                //set constant state bit
                if (blinkMode)  ledState[ledNumber] = 0x05;
                //else clear constant state bit and remember bit
                //set blink bits
                else            ledState[ledNumber] = 0x16;

                }   else    {

                if (blinkMode)  ledState[ledNumber] &= 0x15;    /*clear blink bit */
                else            ledState[ledNumber] &= 0x16;    /* clear constant state bit */

            }

            //if bits 0 and 1 are 0, LED is off so we set ledState to zero
            if (!(ledState[ledNumber] & 3))   ledState[ledNumber] = 0x00;

            break;

            case true:
            //note on event

            //if constant note on is received and LED is already blinking
            //clear blinking bits and set remember bit and constant bit
            if ((!blinkMode) && checkBlinkState(ledNumber))    ledState[ledNumber] = 0x0D;

            //set bit 2 to 1 in any case (constant/blink state)
            else    ledState[ledNumber] |= (0x01 << blinkMode) | 0x04 | (blinkMode << 4);

        }

    }

    if (blinkMode && currentLEDstate)   blinkEnabled = true;
    else    checkBlinkLEDs();

}

void OpenDeck::setLEDState()    {

    bool currentLEDstate;

    //if blinkMode is 1, the LED is blinking
    uint8_t blinkMode = 0;

    /*

    If LED blinking is enabled (as a software feature), LED state is determined by these ranges
    of velocity:

    Velocity 0:         Constant state off
    Velocity 1-62:      Constant state on
    Velocity 63:        Blink state off
    Velocity 64-127:    Blink state on

    If LED blinking is disabled, constant state is turned off by sending velocity 0, and turned on
    by any larger velocity.

    */

    if (bitRead(ledFeatures, SYS_EX_FEATURES_LEDS_BLINK))   {

        if ((receivedVelocity == SYS_EX_LED_VELOCITY_C_OFF) || (receivedVelocity == SYS_EX_LED_VELOCITY_B_OFF))
            currentLEDstate = false;

        else if (

        ((receivedVelocity > SYS_EX_LED_VELOCITY_C_OFF) && (receivedVelocity < SYS_EX_LED_VELOCITY_B_OFF)) ||
        ((receivedVelocity > SYS_EX_LED_VELOCITY_B_OFF) && (receivedVelocity < 128))

        )    currentLEDstate = true;

        else return;

        if ((receivedVelocity >= SYS_EX_LED_VELOCITY_B_OFF) && (receivedVelocity < 128))
            blinkMode = 1;

    }   else    {

            if (receivedVelocity == SYS_EX_LED_VELOCITY_C_OFF)  currentLEDstate = false;
                else                                            currentLEDstate = true;

        }

    handleLED(currentLEDstate, blinkMode, getLEDnumber());

    receivedNoteOnProcessed = true;

}

void OpenDeck::setConstantLEDstate(uint8_t ledNumber)   {

    ledState[ledNumber] = 0x05;

}

void OpenDeck::setBlinkState(uint8_t ledNumber, bool blinkState)    {

    switch (blinkState) {

        case true:
        ledState[ledNumber] |= 0x10;
        break;

        case false:
        ledState[ledNumber] &= 0xEF;
        break;

    }

}

void OpenDeck::switchBlinkState()   {

    if ((millis() - blinkTimerCounter) >= _blinkTime)   {

        //change blinkBit state and write it into ledState variable if LED is in blink state
        for (int i = 0; i<MAX_NUMBER_OF_LEDS; i++)
        if (checkBlinkState(i)) setBlinkState(i, blinkState);

        //invert blink state
        blinkState = !blinkState;

        //update blink timer
        blinkTimerCounter = millis();

    }

}

uint8_t OpenDeck::getLEDnumber()   {

    //match LED activation note with its index
    for (int i=0; i<MAX_NUMBER_OF_LEDS; i++)
        if (ledActNote[i] == receivedNote) return i;

    //since 128 is impossible note, return it in case
    //that received note doesn't match any LED
    return 128;

}

uint8_t OpenDeck::getLEDnote(uint8_t ledNumber)   {

    return ledActNote[ledNumber];

}

bool OpenDeck::checkSameLEDvalue(uint8_t type, uint8_t number)  {

    //do not allow same activation or start-up number for multiple LEDs

    switch(type)    {

        case SYS_EX_MST_LED_ACT_NOTE:
        //led activation note
        for (int i=0; i<MAX_NUMBER_OF_LEDS; i++)
            if (ledActNote[i] == number)    return false;
        break;

        case SYS_EX_MST_LED_START_UP_NUMBER:
        //LED start-up number
        for (int i=0; i<MAX_NUMBER_OF_LEDS; i++)
            if (eeprom_read_byte((uint8_t*)EEPROM_LED_START_UP_NUMBER_START+i) == number)    return false;
        break;

    }   return true;

}