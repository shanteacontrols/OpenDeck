/*

OpenDECK library v1.1
File: OpenDeck.cpp
Last revision date: 2014-09-28
Author: Igor Petrovic

*/


#include "OpenDeck.h"
#include "Ownduino.h"
#include <avr/io.h>
#include <stdlib.h>
#include <avr/eeprom.h>


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

    if (initialEEPROMwrite())   sysExSetDefaultConf();
    else                        getConfiguration(); //get all values from EEPROM

    setNumberOfColumnPasses();

    for (int i=0; i<MAX_NUMBER_OF_BUTTONS; i++)     previousButtonState[i] = buttonDebounceCompare;

    //initialize lastPotNoteValue to 128, which is impossible value for MIDI,
    //to avoid sending note off for that value on first read
    for (int i=0; i<MAX_NUMBER_OF_POTS; i++)        lastPotNoteValue[i] = 128;

    blinkState              = true;
    receivedNoteOnProcessed = true;

    //make initial pot reading to avoid sending all data on startup
    readPots();

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
    _longPressTime                  = 0;
    _blinkTime                      = 0;
    _startUpLEDswitchTime           = 0;

    //software features
    softwareFeatures                = 0;
    startUpRoutinePattern           = 0;

    //hardware features
    hardwareFeatures                = 0;

    //buttons
    for (i=0; i<MAX_NUMBER_OF_BUTTONS; i++)     {

        buttonNote[i]               = 0;
        previousButtonState[i]      = 0;
        longPressState[i]           = 0;

    }

    for (i=0; i<MAX_NUMBER_OF_BUTTONS/8; i++)   {

        buttonType[i]               = 0;
        buttonPressed[i]            = 0;
        longPressSent[i]            = 0;

    }

    buttonDebounceCompare           = 0;

    //pots
    for (i=0; i<MAX_NUMBER_OF_POTS; i++)        {

        ccNumber[i]                 = 0;
        lastPotNoteValue[i]         = 0;
        lastAnalogueValue[i]        = 0;
        potTimer[i]                 = 0;
        ccLowerLimit[i]             = 0;
        ccUpperLimit[i]             = 0;

    }

    for (i=0; i<MAX_NUMBER_OF_POTS/8; i++)      {

        potInverted[i]              = 0;
        potEnabled[i]               = 0;

    }

    _analogueIn                     = 0;

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

    //column counter
    column                          = 0;
    
    //sysex
    sysExEnabled                    = false;

    //board type
    _board                          = 0;

    //free pins
    for (i=0; i<SYS_EX_FREE_PIN_END; i++)
    freePinState[i]                 = 0;
    freePinConfEn                   = false;
    freePinsAsBRows                 = 0;
    freePinsAsLRows                 = 0;

}

void OpenDeck::nextColumn() {

    if (column == _numberOfColumns) column = 0;

    //turn off all LED rows before switching to next column
    if (_board != 0)    {

        ledRowsOff();
        activateColumn(column);

        //increment column
        column++;

    }

}

uint8_t OpenDeck::getActiveColumn() {

    //return currently active column
    return (column - 1);

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


//create instance of library automatically
OpenDeck openDeck;