/*

OpenDeck platform firmware v2.0
Last revision date: 2015-05-08
Author: Igor Petrovic

*/

//disable arduinos hardware serial object
#define HardwareSerial_h

#include <Ownduino.h>
#include "MIDI.h"
#include "OpenDeck.h"


//MIDI callback handlers

void getNoteOnData(uint8_t channel, uint8_t note, uint8_t velocity)  {

    openDeck.storeReceivedNoteOn(channel, note, velocity);

}

void getSysExData(uint8_t *sysExArray, uint8_t size)    {

    openDeck.processSysEx(sysExArray, size);

}


//OpenDeck callback handlers

void sendNotes(uint8_t buttonNote, bool buttonState, uint8_t channel)    {

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

void sendProgramChange(uint8_t channel, uint8_t program)    {

    MIDI.sendProgramChange(program, channel);

}

void sendControlChange(uint8_t ccNumber, uint8_t ccValue, uint8_t channel) {

    MIDI.sendControlChange(ccNumber, ccValue, channel);

}

void sendSysEx(uint8_t *sysExArray, uint8_t size)   {

    MIDI.sendSysEx(size, sysExArray, false);

}

//configure opendeck library
void setOpenDeckHandlers()  {

    openDeck.setHandleNoteSend(sendNotes);
    openDeck.setHandleProgramChangeSend(sendProgramChange);
    openDeck.setHandleControlChangeSend(sendControlChange);
    openDeck.setHandleSysExSend(sendSysEx);

}

//set MIDI handlers
void setMIDIhandlers()  {

    MIDI.setHandleNoteOn(getNoteOnData);
    MIDI.setHandleSystemExclusive(getSysExData);

}

int main()  {

    //setup
    openDeck.init();

    setOpenDeckHandlers();
    setMIDIhandlers();

    SerialOwnduino.begin(38400);

    //read incoming MIDI messages on specified channel
    MIDI.begin(openDeck.getInputMIDIchannel());

    //loop
    while (1)   {

        //check for incoming MIDI messages
        MIDI.read();

        //check buttons
        openDeck.readButtons();

        //check analog inputs
        openDeck.readAnalog();

        //check encoders
        openDeck.readEncoders();

    }

    return 0;

}