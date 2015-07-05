/*

OpenDECK library v1.3
File: OpenDeck.cpp
Last revision date: 2014-12-25
Author: Igor Petrovic

*/


#include "OpenDeck.h"
#include <Ownduino.h>
#include <avr/io.h>
#include <stdlib.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>


OpenDeck::OpenDeck()    {

    //set all callbacks to NULL pointer
    sendNoteCallback            = NULL;
    sendProgramChangeCallback   = NULL;
    sendControlChangeCallback   = NULL;
    sendPitchBendCallback       = NULL;
    sendSysExCallback           = NULL;

}


void OpenDeck::init()   {

    initVariables();

    if (initialEEPROMwrite())   sysExSetDefaultConf();
    else getConfiguration(); //get all values from EEPROM

    boardObject.init();
    readAnalogInitial();

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
    _buttonNoteChannel                      = 0;
    _programChangeChannel                   = 0;
    _analogCCchannel                        = 0;
    _pitchBendChannel                       = 0;
    _inputChannel                           = 0;

    //buttons
    for (i=0; i<MAX_NUMBER_OF_BUTTONS; i++)
        buttonNote[i]                       = 0;

    for (i=0; i<MAX_NUMBER_OF_BUTTONS/8; i++)   {

        buttonType[i]                       = 0;
        buttonPressed[i]                    = 0;
        buttonPCenabled[i]                  = 0;

    }

    //analog
    for (i=0; i<MAX_NUMBER_OF_ANALOG; i++)        {

        analogNumber[i]                     = 0;
        lastAnalogueValue[i]                = 0;
        analogLowerLimit[i]                 = 0;
        analogUpperLimit[i]                 = 0;
        analogDebounceCounter[i]            = 0;
        analogType[i]                       = 0;
        analogTimer[i]                      = 0;

    }

    for (i=0; i<MAX_NUMBER_OF_ANALOG/8; i++)      {

        analogInverted[i]                   = 0;
        analogEnabled[i]                    = 0;

    }

    //encoders
    for (i=0; i<NUMBER_OF_ENCODERS; i++)    {

        lastEncoderState[i]                 = 0;
        initialEncoderDebounceCounter[i]    = 0;
        encoderDirection[i]                 = true;
        lastEncoderSpinTime[i]              = 0;
        encoderNumber[i]                    = 0;
        encoderEnabled[i]                   = 0;
        pulsesPerStep[i]                    = 0;
        encoderInverted[i]                  = 0;
        pulseCounter[i]                     = 0;
        encoderFastMode[i]                  = 0;

    }

    for (i=0; i<NUMBER_OF_BUTTON_COLUMNS; i++)  {

        lastColumnState[i]                  = 0;
        columnPassCounter[i]                = 0;

    }

    //LEDs
    for (i=0; i<MAX_NUMBER_OF_LEDS; i++)        {

        ledActNote[i]                       = 0;

    }

    totalNumberOfLEDs                       = 0;

    //input
    receivedChannel                         = 0;
    receivedNote                            = 0;
    receivedVelocity                        = 0;

    //sysex
    sysExEnabled                    = false;

}

uint8_t OpenDeck::getInputMIDIchannel() {

    return _inputChannel;

}

bool OpenDeck::standardNoteOffEnabled() {

    return bitRead(midiFeatures, SYS_EX_FEATURES_MIDI_STANDARD_NOTE_OFF);

}


//create instance of library automatically
OpenDeck openDeck;