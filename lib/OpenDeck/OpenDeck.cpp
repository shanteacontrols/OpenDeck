/*

OpenDECK library v1.2
File: OpenDeck.cpp
Last revision date: 2014-12-03
Author: Igor Petrovic

*/


#include "OpenDeck.h"
#include "Ownduino.h"
#include <avr/io.h>
#include <stdlib.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>


OpenDeck::OpenDeck()    {

    //initialization
    initVariables();

    //set all callbacks to NULL pointer

    sendButtonDataCallback      =   NULL;
    sendPotCCDataCallback       =   NULL;
    sendPotNoteOnDataCallback   =   NULL;
    sendPotNoteOffDataCallback  =   NULL;
    sendSysExDataCallback       =   NULL;

}


void OpenDeck::init()   {

    setADCprescaler(32);
    set8bitADC();

    if (initialEEPROMwrite())   sysExSetDefaultConf();
    else                        getConfiguration(); //get all values from EEPROM

    setNumberOfColumnPasses();

    //initialize lastPotNoteValue to 128, which is impossible value for MIDI,
    //to avoid sending note off for that value on first read
    for (int i=0; i<MAX_NUMBER_OF_POTS; i++)        lastPotNoteValue[i] = 128;

    blinkState              = true;
    receivedNoteOnProcessed = true;

    //make initial pot reading to avoid sending all data on startup
    readPotsInitial();

    //configure column and analog pin switch timer
    setUpSwitchTimer();

    //run LED animation on start-up
    startUpRoutine();

}

bool OpenDeck::initialEEPROMwrite()  {

    //if ID bytes haven't been written to EEPROM on specified address,
    //write default configuration to EEPROM
    if  (!(

        (eeprom_read_byte((uint8_t*)EEPROM_M_ID_BYTE_0) == SYS_EX_M_ID_0) &&
        (eeprom_read_byte((uint8_t*)EEPROM_M_ID_BYTE_1) == SYS_EX_M_ID_1) &&
        (eeprom_read_byte((uint8_t*)EEPROM_M_ID_BYTE_2) == SYS_EX_M_ID_2)

    ))   return true; return false;

}

void OpenDeck::initVariables()  {

    //reset all variables

    //MIDI channels
    _buttonNoteChannel              = 0;
    _longPressButtonNoteChannel     = 0;
    _potCCchannel                   = 0;
    _potNoteChannel                 = 0;
    _encCCchannel                   = 0;
    _inputChannel                   = 0;

    //hardware params
    _blinkTime                      = 0;

    //software features
    softwareFeatures                = 0;

    //hardware features
    hardwareFeatures                = 0;

    //buttons
    for (i=0; i<MAX_NUMBER_OF_BUTTONS; i++)
        buttonNote[i]               = 0;

    for (i=0; i<MAX_NUMBER_OF_BUTTONS/8; i++)   {

        buttonType[i]               = 0;
        buttonPressed[i]            = 0;
        longPressSent[i]            = 0;
        longPressCounter[i]         = 0;

    }

    numberOfColumnPasses            = 0;
    longPressColumnPass             = 0;

    //pots
    for (i=0; i<MAX_NUMBER_OF_POTS; i++)        {

        ccNumber[i]                 = 0;
        lastPotNoteValue[i]         = 0;
        lastAnalogueValue[i]        = 0;
        ccLowerLimit[i]             = 0;
        ccUpperLimit[i]             = 0;

    }

    for (i=0; i<MAX_NUMBER_OF_POTS/8; i++)      {

        potInverted[i]              = 0;
        potEnabled[i]               = 0;

    }

    for (i=0; i<8; i++)                         {

        lastColumnState[i] = 0;
        columnPassCounter[i] = 0;
        analogueEnabledArray[i] = 0;

    }

    //LEDs
    for (i=0; i<MAX_NUMBER_OF_LEDS; i++)        {

        ledState[i]                 = 0;
        ledNote[i]                    = 0;

    }

    totalNumberOfLEDs               = 0;

    blinkState                      = false;
    blinkEnabled                    = false;
    blinkTimerCounter               = 0;

    //input
    receivedNoteOnProcessed         = false;
    receivedChannel                 = 0;
    receivedNote                    = 0;
    receivedVelocity                = 0;

    //sysex
    sysExEnabled                    = false;

    //board type
    _board                          = 0;

    //encoders
    for (int i=0; i<MAX_NUMBER_OF_ENCODERS/8; i++)   {

        encoderPairEnabled[i]   = 0;

    }

    for (int i=0; i<MAX_NUMBER_OF_ENCODERS; i++)
        encoderPairState[i]         = 0;

}

void OpenDeck::setUpSwitchTimer()   {

    /*

        This timer is used to switch columns in matrix, and also
        to switch analogue input pin on ATmega328p. It's configured
        to run every 500 microseconds. Interrupt routine either switches
        matrix column, or analogue pin, which results in column/analog pin
        switching every 1ms.

    */

    TCCR2A = 0;
    TCCR2B = 0;
    TCNT2  = 0;

    //turn on CTC mode
    TCCR2A |= (1 << WGM21);

    //set prescaler to 64
    TCCR2B |= (1 << CS22);

    //set compare match register to desired timer count
    OCR2A = 124;

    //enable CTC interrupt
    TIMSK2 |= (1 << OCIE2A);

}

void OpenDeck::processMatrix()  {

    static int8_t previousColumn = -1;
    int8_t currentColumn = getActiveColumn();

    if (currentColumn != previousColumn)    {

        //if any of the LEDs on current
        //column are active, turn them on
        checkLEDs(currentColumn);

        //check buttons on current column
        readButtons(currentColumn);

        previousColumn = currentColumn;

    }

}

uint8_t OpenDeck::getNumberOfColumns()  {

    return _numberOfColumns;

}

uint8_t OpenDeck::getNumberOfMux()    {

    return _numberOfMux;

}

uint8_t OpenDeck::getBoard()    {

    return _board;

}

uint8_t OpenDeck::getInputMIDIchannel() {

    return _inputChannel;

}

bool OpenDeck::standardNoteOffEnabled() {

    return bitRead(softwareFeatures, EEPROM_SW_F_STANDARD_NOTE_OFF);

}

bool OpenDeck::runningStatusEnabled()   {

    return bitRead(softwareFeatures, EEPROM_SW_F_RUNNING_STATUS);

}

void OpenDeck::stopSwitchTimer()    {

    TIMSK2 &= (0 << OCIE2A);

}


//create instance of library automatically
OpenDeck openDeck;