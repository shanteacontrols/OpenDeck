/*

Tannin MIDI controller firmware v2.2
Last revision date: 2014-09-03
Author: Igor Petrovic

*/

#include "OpenDeck.h"
#include "MIDI.h"
#include "Ownduino.h"
#include <avr/io.h>

//velocity for on and off events
#define MIDI_NOTE_ON_VELOCITY   127
#define MIDI_NOTE_OFF_VELOCITY  0

//define time after which code in timedLoop function runs
#define COLUMN_SWITCH_TIME 1


//MIDI callback handlers

//receive and store note on data
void getNoteOnData(uint8_t channel, uint8_t pitch, uint8_t velocity)  {

    openDeck.storeReceivedNote(channel, pitch, velocity);

}

void getSysExData(uint8_t *sysExArray, uint8_t size)    {

    openDeck.processSysEx(sysExArray, size);

}


//OpenDeck callback handlers

//send button note midi data
void sendButtonData(uint8_t buttonNumber, bool buttonState, uint8_t channel)    {

    //see checkButton in OpenDeck.cpp

    switch (buttonState) {

        case false:
        //button released
        if (openDeck.standardNoteOffEnabled())  MIDI.sendNoteOff(buttonNumber, MIDI_NOTE_OFF_VELOCITY, channel);
        else                                    MIDI.sendNoteOn(buttonNumber, MIDI_NOTE_OFF_VELOCITY, channel);
        break;

        case true:
        //button pressed
        MIDI.sendNoteOn(buttonNumber, MIDI_NOTE_ON_VELOCITY, channel);
        break;

    }

}

//send pot CC midi data
void sendPotCCData(uint8_t potNumber, uint8_t ccValue, uint8_t channel) {

    MIDI.sendControlChange(potNumber, ccValue, channel);

}

//send pot note on midi data
void sendPotNoteOnData(uint8_t note, uint8_t potNumber, uint8_t channel)    {

    MIDI.sendNoteOn(note, MIDI_NOTE_ON_VELOCITY, channel);

}

//send pot note off midi data
void sendPotNoteOffData(uint8_t note, uint8_t potNumber, uint8_t channel)   {

    if (openDeck.standardNoteOffEnabled())  MIDI.sendNoteOff(note, MIDI_NOTE_OFF_VELOCITY, channel);
    else                                    MIDI.sendNoteOn(note, MIDI_NOTE_OFF_VELOCITY, channel);

}

void sendSysExData(uint8_t *sysExArray, uint8_t size)   {

    MIDI.sendSysEx(size, sysExArray, false);

}


//configure opendeck library
void setOpenDeckHandlers()  {

    openDeck.setHandleButtonSend(sendButtonData);

    openDeck.setHandlePotCC(sendPotCCData);
    openDeck.setHandlePotNoteOn(sendPotNoteOnData);
    openDeck.setHandlePotNoteOff(sendPotNoteOffData);

    openDeck.setHandleSysExSend(sendSysExData);

}

//set MIDI handlers
void setMIDIhandlers()  {

    MIDI.setHandleNoteOn(getNoteOnData);
    MIDI.setHandleSystemExclusive(getSysExData);

}


//main
void setup()  {

    //set time in ms after which timedLoop is run
    setTimedLoop(COLUMN_SWITCH_TIME);

    setOpenDeckHandlers();
    setMIDIhandlers();

    //write default controller settings to EEPROM
    //openDeck.setDefaultConf();

    //initialize openDeck library
    openDeck.setBoard(BOARD_OPEN_DECK_1);

    openDeck.init();

    //run LED animation on start-up
    openDeck.startUpRoutine();

    //read incoming MIDI messages on specified channel
    MIDI.begin(openDeck.getInputMIDIchannel());

    Serial.begin(38400);

}

void timedLoop()    {

    //activate next column
    openDeck.nextColumn();

    //if any of the LEDs on current
    //column are active, turn them on
    if (openDeck.ledsEnabled())     openDeck.checkLEDs();

    //check buttons on current column
    if (openDeck.buttonsEnabled())  openDeck.readButtons();

}

void loop() {

    //constantly check for incoming MIDI messages
    MIDI.read();

    //check if there is any received note to process
    openDeck.checkReceivedNote();

    //read all pots
    if (openDeck.potsEnabled())     openDeck.readPots();

}