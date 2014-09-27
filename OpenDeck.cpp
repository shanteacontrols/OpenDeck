/*

OpenDeck platform firmware v1.1
Last revision date: 2014-09-28
Author: Igor Petrovic

*/

#include "OpenDeck.h"
#include "MIDI.h"
#include "Ownduino.h"
#include <avr/io.h>
#include <avr/eeprom.h>

//velocity for on and off events
#define MIDI_NOTE_ON_VELOCITY   127
#define MIDI_NOTE_OFF_VELOCITY  0

//define time after which code in timedLoop function runs
#define COLUMN_SWITCH_TIME 1


//MIDI callback handlers

void getNoteOnData(uint8_t channel, uint8_t note, uint8_t velocity)  {

    openDeck.storeReceivedNoteOn(channel, note, velocity);

}

void getSysExData(uint8_t *sysExArray, uint8_t size)    {

    openDeck.processSysEx(sysExArray, size);

}


//OpenDeck callback handlers

void sendButtonData(uint8_t buttonNote, bool buttonState, uint8_t channel)    {

    switch (buttonState) {

        case false:
        //button released
        if (openDeck.standardNoteOffEnabled())  MIDI.sendNoteOff(buttonNote, MIDI_NOTE_OFF_VELOCITY, channel);
        else                                    MIDI.sendNoteOn(buttonNote, MIDI_NOTE_OFF_VELOCITY, channel);
        break;

        case true:
        //button pressed
        MIDI.sendNoteOn(buttonNote, MIDI_NOTE_ON_VELOCITY, channel);
        break;

    }

}

void sendPotCCData(uint8_t ccNumber, uint8_t ccValue, uint8_t channel) {

    MIDI.sendControlChange(ccNumber, ccValue, channel);

}

void sendPotNoteOnData(uint8_t note, uint8_t channel)    {

    MIDI.sendNoteOn(note, MIDI_NOTE_ON_VELOCITY, channel);

}

void sendPotNoteOffData(uint8_t note, uint8_t channel)   {

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

    //initialize openDeck library for specified board
    openDeck.init();

    setOpenDeckHandlers();
    setMIDIhandlers();

    //read incoming MIDI messages on specified channel
    MIDI.begin(openDeck.getInputMIDIchannel(), openDeck.runningStatusEnabled());

    Serial.begin(31250);

}

void timedLoop()    {

    //activate next column
    openDeck.nextColumn();

    //if any of the LEDs on current
    //column are active, turn them on
    openDeck.checkLEDs();

    //check buttons on current column
    openDeck.readButtons();

}

void loop() {

    //constantly check for incoming MIDI messages
    MIDI.read();

    //check if there is any received note to process
    openDeck.checkReceivedNoteOn();

    //read all pots
    openDeck.readPots();

}