/*

OpenDeck MIDI controller firmware v1.93
Last revision date: 2014-08-27
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
#define COLUMN_SWITCH_TIME  1


//hardware control

//initialize pins
void initPins() {

    //set in/out pins
    DDRD = 0x02;
    DDRB = 0x0F;
    DDRC = 0x3F;

    //enable internal pull-up resistors for button rows
    PORTD = 0xFC;

    //select first column
    PORTC = 0x00;

}

//switch to next matrix column
void activateColumn(uint8_t column) {

    //column switching is controlled by 74HC238 decoder
    PORTC &= 0xC7;
    PORTC |= (0xC7 | (column << 3));

}

//turn led row on
void ledRowOn(uint8_t rowNumber)    {

    switch (rowNumber)  {

        case 0:
        //turn on first LED row
        PORTB |= 0x01;
        break;

        case 1:
        //turn on second LED row
        PORTB |= 0x02;
        break;

        case 2:
        //turn on third LED row
        PORTB |= 0x04;
        break;

        case 3:
        //turn on fourth LED row
        PORTB |= 0x08;
        break;

        default:
        break;

    }

}

//turn all LED rows off
void ledRowsOff()   {

    PORTB &= 0xF0;

}

//control select pins on mux
void setMuxOutput(uint8_t muxInput) {

    PORTC &= 0xF8;
    PORTC |= muxInput;

}

//get button readings from all rows in matrix
void readButtons(uint8_t &buttonColumnState)    {

    buttonColumnState = ((PIND >> 4) & 0x0F);

}



//MIDI callback handlers

//receive and store note on data
void getNoteOnData(uint8_t channel, uint8_t pitch, uint8_t velocity)    {

    openDeck.storeReceivedNote(channel, pitch, velocity);

}

void getSysExData(uint8_t *sysExArray, uint8_t size)    {

    openDeck.processSysEx(sysExArray, size);

}


//OpenDeck callback handlers

//send button note midi data
void sendButtonData(uint8_t buttonNumber, bool buttonState, uint8_t channel)    {

    //see checkButton in OpenDeck.cpp

    switch (buttonState)    {

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


//start-up animation
void startUpRoutine()  {

    openDeck.oneByOneLED(true, true, true);
    openDeck.oneByOneLED(false, false, true);
    openDeck.oneByOneLED(true, false, false);
    openDeck.oneByOneLED(false, true, true);
    openDeck.oneByOneLED(true, false, true);
    openDeck.oneByOneLED(false, false, false);
    openDeck.allLEDsOff();

}



//configure opendeck library
void setOpenDeckHandlers()  {

    openDeck.setNumberOfMux(2);
    openDeck.setNumberOfColumns(8);
    openDeck.setNumberOfButtonRows(4);
    openDeck.setNumberOfLEDrows(4);

    openDeck.enableAnalogueInput(6);
    openDeck.enableAnalogueInput(7);

    openDeck.setHandleButtonSend(sendButtonData);

    openDeck.setHandlePotCC(sendPotCCData);
    openDeck.setHandlePotNoteOn(sendPotNoteOnData);
    openDeck.setHandlePotNoteOff(sendPotNoteOffData);

    openDeck.setHandleSysExSend(sendSysExData);

    openDeck.setHandlePinInit(initPins);
    openDeck.setHandleColumnSwitch(activateColumn);
    openDeck.setHandleLEDrowOn(ledRowOn);
    openDeck.setHandleLEDrowsOff(ledRowsOff);
    openDeck.setHandleMuxOutput(setMuxOutput);
    openDeck.setHandleButtonRead(readButtons);

}

//set MIDI handlers
void setMIDIhandlers()  {

    MIDI.setHandleNoteOn(getNoteOnData);
    MIDI.setHandleSystemExclusive(getSysExData);

}


//main
void setup()    {

    //set time in ms after which timedLoop is run
    setTimedLoop(COLUMN_SWITCH_TIME);

    setOpenDeckHandlers();
    setMIDIhandlers();

    //write default controller settings to EEPROM
    //openDeck.setDefaultConf();

    //initialize openDeck library
    openDeck.init();

    //run LED animation on start-up
    startUpRoutine();

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