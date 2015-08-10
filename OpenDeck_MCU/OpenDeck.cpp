/*

OpenDECK library v1.3
File: OpenDeck.cpp
Last revision date: 2014-12-25
Author: Igor Petrovic

*/

#include "OpenDeck.h"
#include <avr/eeprom.h>

void storeReceivedNoteOn(uint8_t channel, uint8_t note, uint8_t velocity)  {

    openDeck.receivedNote = note;
    openDeck.receivedVelocity = velocity;

    openDeck.setLEDState();

}

void processSysEx(uint8_t sysExArray[], uint8_t arrSize, sysExSource messageSource)  {

    openDeck.setSysExSource(messageSource);

    if (openDeck.sysExCheckMessageValidity(sysExArray, arrSize))
        openDeck.sysExGenerateResponse(sysExArray, arrSize);

}


OpenDeck::OpenDeck()    {

    //default constructor

}


void OpenDeck::init()   {

    initVariables();

    boardObject.init();
    readAnalogInitial();

    if (initialEEPROMwrite())   {

        clearEEPROM();
        sysExSetDefaultConf();

    } else getConfiguration(); //get all values from EEPROM

    //run LED animation on start-up
    startUpRoutine();

    #ifdef HW_MIDI
        //read incoming MIDI messages on specified channel
        MIDI.begin(_inputChannel);

        MIDI.setHandleNoteOn(storeReceivedNoteOn);
        MIDI.setHandleSystemExclusive(processSysEx);
    #endif

    #ifdef USBMIDI
        usbMIDI.setHandleNoteOn(storeReceivedNoteOn);
    #endif

}

bool OpenDeck::initialEEPROMwrite()  {

    //if ID bytes haven't been written to EEPROM on specified address,
    //write default configuration to EEPROM
    if  (!(

    (eeprom_read_byte((uint8_t*)ID_LOCATION_0) == UNIQUE_ID) &&
    (eeprom_read_byte((uint8_t*)ID_LOCATION_1) == UNIQUE_ID) &&
    (eeprom_read_byte((uint8_t*)ID_LOCATION_2) == UNIQUE_ID)

    ))   return true; return false;

}

void OpenDeck::initVariables()  {

    //reset all variables

    //MIDI channels
    _noteChannel                            = 0;
    _programChangeChannel                   = 0;
    _CCchannel                              = 0;
    _pitchBendChannel                       = 0;
    _inputChannel                           = 0;

    //buttons
    for (i=0; i<MAX_NUMBER_OF_BUTTONS; i++)
        noteNumber[i]                       = 0;

    for (i=0; i<MAX_NUMBER_OF_BUTTONS/8+1; i++)       {

        _buttonType[i]                       = 0;
        buttonPressed[i]                    = 0;
        buttonPCenabled[i]                  = 0;

    }

    //analog
    for (i=0; i<MAX_NUMBER_OF_ANALOG; i++)          {

        ccNumber[i]                         = 0;
        lastAnalogueValue[i]                = 0;
        analogLowerLimit[i]                 = 0;
        analogUpperLimit[i]                 = 0;
        analogDebounceCounter[i]            = 0;
        analogType[i]                       = 0;

    }

    for (i=0; i<MAX_NUMBER_OF_ANALOG/8+1; i++)        {

        analogInverted[i]                   = 0;
        analogEnabled[i]                    = 0;

    }

    //encoders
    for (i=0; i<MAX_NUMBER_OF_ENCODERS; i++)            {

        initialEncoderDebounceCounter[i]    = 0;
        lastEncoderSpinTime[i]              = 0;
        encoderNumber[i]                    = 0;
        pulsesPerStep[i]                    = 0;

    }

    for (i=0; i<NUMBER_OF_ENCODERS/8+1; i++)        {

        encoderEnabled[i]                   = 0;
        encoderInverted[i]                  = 0;
        encoderFastMode[i]                  = 0;

    }

    for (i=0; i<NUMBER_OF_BUTTON_COLUMNS; i++)      {

        lastColumnState[i]                  = 0;
        columnPassCounter[i]                = 0;

    }

    //LEDs
    for (i=0; i<MAX_NUMBER_OF_LEDS; i++)            {

        ledActNote[i]                       = 0;

    }

    totalNumberOfLEDs                       = 0;

    //input
    receivedNote                            = 0;
    receivedVelocity                        = 0;

    //sysex
    sysExEnabled                            = false;
    sysExMessageSource                      = usbSource;

}

bool OpenDeck::standardNoteOffEnabled() {

    return bitRead(midiFeatures, SYS_EX_FEATURES_MIDI_STANDARD_NOTE_OFF);

}

void OpenDeck::checkMIDIIn()    {

    #ifdef USBMIDI
        static bool sysExProcessed = true;

        if (usbMIDI.read(_inputChannel))   {

            sysExProcessed = false;

            } else {

            uint8_t messageType = usbMIDI.getType();

            //new message has arrived
            if (!sysExProcessed && (messageType == 7)) {

                processSysEx(usbMIDI.getSysExArray(), usbMIDI.getData1(), usbSource);
                sysExProcessed = true;

            }

        }
    #endif

    //check for incoming MIDI messages on USART
    #ifdef HW_MIDI
        MIDI.read();
    #endif

}


//create instance of library automatically
OpenDeck openDeck;