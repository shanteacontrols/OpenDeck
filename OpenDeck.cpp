/*

OpenDeck platform firmware v1.3
Last revision date: 2014-12-25
Author: Igor Petrovic

*/

#include "OpenDeck.h"
#include "MIDI.h"
#include "Ownduino.h"
//#include "Encoder.h"
#include <avr/eeprom.h>


//velocity for on and off events
#define MIDI_NOTE_ON_VELOCITY   127
#define MIDI_NOTE_OFF_VELOCITY  0

//Encoder encoder1 = Encoder(PIN_C,PIN_D);


//MIDI callback handlers

void getNoteOnData(uint8_t channel, uint8_t note, uint8_t velocity)  {

    openDeck.storeReceivedNoteOn(channel, note, velocity);

}

void getSysExData(uint8_t *sysExArray, uint8_t size)    {

    openDeck.processSysEx(sysExArray, size);

}


//OpenDeck callback handlers

void sendButtonNotes(uint8_t buttonNote, bool buttonState, uint8_t channel)    {

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

void sendButtonPP(uint8_t channel, uint8_t program)    {

    MIDI.sendProgramChange(program, channel);

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

    openDeck.setHandleButtonNoteSend(sendButtonNotes);
    openDeck.setHandleButtonPPSend(sendButtonPP);

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

    openDeck.init();

    setOpenDeckHandlers();
    setMIDIhandlers();

    //read incoming MIDI messages on specified channel
    //disable running status by default
    MIDI.begin(openDeck.getInputMIDIchannel(), false);

    Serial.begin(38400);

}

void loop() {

    //process buttons and LEDs
    openDeck.processMatrix();

    //constantly check for incoming MIDI messages
    MIDI.read();

    //check if there is any received note to process
    openDeck.checkReceivedNoteOn();

    //read pots one analogue input at the time
    openDeck.readPots();

    //openDeck.readEncoders(encoder1.read());

}