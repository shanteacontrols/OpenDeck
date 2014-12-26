/*

OpenDECK library v1.3
File: OpenDeck.cpp
Last revision date: 2014-12-25
Author: Igor Petrovic

*/


#include "OpenDeck.h"
#include "Ownduino.h"
#include <avr/io.h>
#include <stdlib.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>


OpenDeck::OpenDeck()    {

    //set all callbacks to NULL pointer
    sendButtonNoteDataCallback  =   NULL;
    sendButtonPPDataCallback    =   NULL;
    sendPotCCDataCallback       =   NULL;
    sendPotNoteOnDataCallback   =   NULL;
    sendPotNoteOffDataCallback  =   NULL;
    sendSysExDataCallback       =   NULL;

}


void OpenDeck::init()   {

    initVariables();

    setADCprescaler(32);
    set8bitADC();

    if (initialEEPROMwrite())   sysExSetDefaultConf();
    else getConfiguration(); //get all values from EEPROM

    initBoard();

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
    _buttonPPchannel                = 0;
    _potCCchannel                   = 0;
    _potPPchannel                   = 0;
    _potNoteChannel                 = 0;
    _inputChannel                   = 0;

    //hardware params
    _blinkTime                      = 0;

    //buttons
    for (i=0; i<MAX_NUMBER_OF_BUTTONS; i++)
        buttonNote[i]               = 0;

    for (i=0; i<MAX_NUMBER_OF_BUTTONS/8; i++)   {

        buttonType[i]               = 0;
        buttonPressed[i]            = 0;
        longPressSent[i]            = 0;
        longPressCounter[i]         = 0;
        buttonPPenabled[i]          = 0;

    }

    numberOfColumnPasses            = 0;
    longPressColumnPass             = 0;

    //pots
    for (i=0; i<MAX_NUMBER_OF_POTS; i++)        {

        ccppNumber[i]               = 0;
        lastPotNoteValue[i]         = 128;
        lastAnalogueValue[i]        = 0;
        ccLowerLimit[i]             = 0;
        ccUpperLimit[i]             = 0;

    }

    for (i=0; i<MAX_NUMBER_OF_POTS/8; i++)      {

        potInverted[i]              = 0;
        potEnabled[i]               = 0;
        potPPenabled[i]             = 0;

    }

    for (i=0; i<8; i++)                         {

        lastColumnState[i] = 0;
        columnPassCounter[i] = 0;
        analogueEnabledArray[i] = 0;

    }

    //LEDs
    for (i=0; i<MAX_NUMBER_OF_LEDS; i++)        {

        ledState[i]                 = 0;
        ledActNote[i]               = 0;

    }

    totalNumberOfLEDs               = 0;

    blinkState                      = true;
    blinkEnabled                    = false;
    blinkTimerCounter               = 0;

    //input
    receivedNoteOnProcessed         = true;
    receivedChannel                 = 0;
    receivedNote                    = 0;
    receivedVelocity                = 0;

    //sysex
    sysExEnabled                    = false;

    //board type
    _board                          = 0;

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

    return bitRead(midiFeatures, SYS_EX_FEATURES_MIDI_STANDARD_NOTE_OFF);

}

bool OpenDeck::runningStatusEnabled()   {

    return bitRead(midiFeatures, SYS_EX_FEATURES_MIDI_RUNNING_STATUS);

}

void OpenDeck::stopSwitchTimer()    {

    TIMSK2 &= (0 << OCIE2A);

}


//create instance of library automatically
OpenDeck openDeck;