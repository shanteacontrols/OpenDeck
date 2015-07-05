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

    switch (eeprom_read_byte((uint8_t*)EEPROM_LEDS_HW_P_START_UP_ROUTINE))  {

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

bool OpenDeck::ledOn(uint8_t ledNumber) {

    return boardObject.getLEDstate(ledNumber);

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

    uint16_t startUpLEDswitchTime = eeprom_read_byte((uint8_t*)EEPROM_LEDS_HW_P_START_UP_SWITCH_TIME) * 10;

    //while loop counter
    uint8_t passCounter = 0;

    //reset the timer on each function call
    uint32_t startUpTimer = 0;

    //index of LED to be processed next
    uint8_t ledNumber,
            _ledNumber[MAX_NUMBER_OF_LEDS];

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

            //only process LED after defined time
            if ((millisOwnduino() - startUpTimer) > startUpLEDswitchTime)  {

                if (passCounter < totalNumberOfLEDs)    {

                    //if we're turning LEDs on one by one, turn all the other LEDs off
                    if (singleLED && turnOn)            allLEDsOff();

                    //if we're turning LEDs off one by one, turn all the other LEDs on
                    else    if (!turnOn && singleLED)   allLEDsOn();

                    //set LED state depending on turnOn parameter
                    if (turnOn) boardObject.turnOnLED(ledNumber);
                        else    boardObject.turnOffLED(ledNumber);

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
            startUpTimer = millisOwnduino();

        }

    }

}

void OpenDeck::allLEDsOn()  {

    //turn on all LEDs
    for (int i=0; i<MAX_NUMBER_OF_LEDS; i++)    boardObject.turnOnLED(i);

}

void OpenDeck::allLEDsOff() {

    //turn off all LEDs
    for (int i=0; i<MAX_NUMBER_OF_LEDS; i++)    boardObject.turnOffLED(i);

}

void OpenDeck::storeReceivedNoteOn(uint8_t channel, uint8_t note, uint8_t velocity)  {

    receivedChannel = channel;
    receivedNote = note;
    receivedVelocity = velocity;

    setLEDState();

}

bool OpenDeck::checkLEDsOn()    {

    //return true if all LEDs are on
    for (int i=0; i<MAX_NUMBER_OF_LEDS; i++)    if (boardObject.getLEDstate(i))   return false;
    return true;

}

bool OpenDeck::checkLEDsOff()   {

    //return true if all LEDs are off
    for (int i=0; i<MAX_NUMBER_OF_LEDS; i++)    if (!boardObject.getLEDstate(i))   return false;
    return true;

}

void OpenDeck::setLEDState()    {

    bool currentLEDstate;

    //if blinkMode is 1, the LED is blinking
    uint8_t blinkMode = 0;

        if ((receivedVelocity == SYS_EX_LED_VELOCITY_C_OFF) || (receivedVelocity == SYS_EX_LED_VELOCITY_B_OFF))
            currentLEDstate = false;

        else if (

        ((receivedVelocity > SYS_EX_LED_VELOCITY_C_OFF) && (receivedVelocity < SYS_EX_LED_VELOCITY_B_OFF)) ||
        ((receivedVelocity > SYS_EX_LED_VELOCITY_B_OFF) && (receivedVelocity < 128))

        )    currentLEDstate = true;

        else return;

        if ((receivedVelocity >= SYS_EX_LED_VELOCITY_B_OFF) && (receivedVelocity < 128))
            blinkMode = 1;

    boardObject.handleLED(currentLEDstate, blinkMode, getLEDid());

}

uint8_t OpenDeck::getLEDid()   {

    //match LED activation note with its index
    for (int i=0; i<MAX_NUMBER_OF_LEDS; i++)
        if (ledActNote[i] == receivedNote) return i;

    //since 128 is impossible note, return it in case
    //that received note doesn't match any LED
    return 128;

}

uint8_t OpenDeck::getLEDActivationNote(uint8_t ledNumber)   {

    return ledActNote[ledNumber];

}

bool OpenDeck::checkSameLEDvalue(uint8_t type, uint8_t number)  {

    //do not allow same activation or start-up number for multiple LEDs

    switch(type)    {

        case SYS_EX_MST_LEDS_ACT_NOTE:
        //led activation note
        for (int i=0; i<MAX_NUMBER_OF_LEDS; i++)
            if (ledActNote[i] == number)    return false;
        break;

        case SYS_EX_MST_LEDS_START_UP_NUMBER:
        //LED start-up number
        for (int i=0; i<MAX_NUMBER_OF_LEDS; i++)
            if (eeprom_read_byte((uint8_t*)EEPROM_LEDS_START_UP_NUMBER_START+i) == number)    return false;
        break;

    }   return true;

}