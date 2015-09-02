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

    boardObject.init();
    initVariables();

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
        usbMIDI.setHandleNoteOff(storeReceivedNoteOn);
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

    for (i=0; i<MAX_NUMBER_OF_BUTTONS/8+1; i++)
        buttonPressed[i]                    = 0;

    //analog
    for (i=0; i<MAX_NUMBER_OF_ANALOG; i++)          {

        lastAnalogueValue[i]                = 0;
        analogDebounceCounter[i]            = 0;

    }

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