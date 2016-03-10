#include "MIDI.h"
#include "../../sysex/SysEx.h"
#include "../../eeprom/Configuration.h"
#include "../../BitManipulation.h"
#include "../../hardware/midi/usb_midi/USBmidi.h"
#include "../../hardware/board/Board.h"
#include "../../interface/leds/LEDs.h"

MIDI::MIDI()    {

    //default constructor

}

void MIDI::init() {

    const subtype midiFeatureSubtype = { MIDI_FEATURES, 0, 1 };
    const subtype midiChannelSubtype = { MIDI_CHANNELS, 1, 16 };

    const subtype *midiSubtypeArray[] = {

        &midiFeatureSubtype,
        &midiChannelSubtype

    };

    sysEx.addMessageType(CONF_MIDI_BLOCK, MIDI_SUBTYPES);

    for (int i=0; i<MIDI_SUBTYPES; i++)   {

        //define subtype messages
        sysEx.addMessageSubType(CONF_MIDI_BLOCK, i, midiSubtypeArray[i]->parameters, midiSubtypeArray[i]->lowValue, midiSubtypeArray[i]->highValue);

    }

    uint8_t inChannel = getMIDIchannel(inputChannel);
    //read incoming MIDI messages on specified channel
    hwMIDI.begin(inChannel);
    usbMIDI.begin(inChannel);

}

void MIDI::checkInput()   {

    if (usbMIDI.read())   {   //new message on usb

        uint8_t messageType = usbMIDI.getType();
        source = usbSource;

        switch(messageType) {

            case midiMessageSystemExclusive:
            sysEx.handleSysEx(usbMIDI.getSysExArray(), usbMIDI.getData1());
            lastSysExMessageTime = rTimeMillis();
            break;

            case midiMessageNoteOff:
            case midiMessageNoteOn:
            //we're using received note data to control LEDs
            leds.noteToLEDstate(usbMIDI.getData1(), usbMIDI.getData2());
            break;

        }

    }

    //check for incoming MIDI messages on USART
    if (hwMIDI.read())    {

        uint8_t messageType = hwMIDI.getType();
        uint8_t data1 = hwMIDI.getData1();
        uint8_t data2 = hwMIDI.getData2();

        source = midiSource;

        if (!getFeature(midiFeatureUSBconvert))  {

            switch(messageType) {

                case midiMessageNoteOff:
                case midiMessageNoteOn:
                leds.noteToLEDstate(data1, data2);
                break;

                default:
                break;

            }

        }   else {

                //dump everything from MIDI in to USB MIDI out
                switch(messageType) {

                    case midiMessageNoteOff:
                    usbMIDI.sendNoteOn(data1, data2, getMIDIchannel(inputChannel));
                    break;

                    case midiMessageNoteOn:
                    usbMIDI.sendNoteOn(data1, data2, getMIDIchannel(inputChannel));
                    break;

                    case midiMessageControlChange:
                    usbMIDI.sendControlChange(data1, data2, getMIDIchannel(inputChannel));
                    break;

                    case midiMessageProgramChange:
                    usbMIDI.sendProgramChange(data1, getMIDIchannel(inputChannel));
                    break;

                    case midiMessageSystemExclusive:
                    usbMIDI.sendSysEx(hwMIDI.getSysExArrayLength(), hwMIDI.getSysExArray(), false);
                    break;

                    case midiMessageAfterTouchChannel:
                    usbMIDI.sendAfterTouch(data1, getMIDIchannel(inputChannel));
                    break;

                    case midiMessagePitchBend:
                    //to-do
                    break;

                }

        }

    }

    //disable sysex config after inactivity
    if (rTimeMillis() - lastSysExMessageTime > CONFIG_TIMEOUT)
        sysEx.disableConf();

}

void MIDI::sendMIDInote(uint8_t note, bool state, uint8_t _velocity)  {

    uint8_t channel = getMIDIchannel(noteChannel);

    switch (state) {

        case false:
        //button released
        if (getFeature(midiFeatureStandardNoteOff))   {

            usbMIDI.sendNoteOff(note, _velocity, channel);
            hwMIDI.sendNoteOff(note, _velocity, channel);

        } else {

            usbMIDI.sendNoteOn(note, _velocity, channel);
            hwMIDI.sendNoteOn(note, _velocity, channel);

        }
        break;

        case true:
        //button pressed
        usbMIDI.sendNoteOn(note, _velocity, channel);
        hwMIDI.sendNoteOn(note, _velocity, channel);
        break;

    }

}

void MIDI::sendProgramChange(uint8_t program)    {

    uint8_t channel = getMIDIchannel(programChangeChannel);
    usbMIDI.sendProgramChange(program, channel);
    hwMIDI.sendProgramChange(program, channel);

}

void MIDI::sendControlChange(uint8_t ccNumber, uint8_t ccValue) {

    uint8_t channel = getMIDIchannel(CCchannel);
    usbMIDI.sendControlChange(ccNumber, ccValue, channel);
    hwMIDI.sendControlChange(ccNumber, ccValue, channel);

}

void MIDI::sendSysEx(uint8_t *sysExArray, uint8_t arraySize)   {

    switch (source) {

        case midiSource:
        hwMIDI.sendSysEx(arraySize, sysExArray, false);
        break;

        case usbSource:
        usbMIDI.sendSysEx(arraySize, sysExArray, false);
        break;

    }

}


bool MIDI::getFeature(uint8_t featureID)  {

    return configuration.readParameter(CONF_MIDI_BLOCK, midiFeatureSection, featureID);

}

uint8_t MIDI::getMIDIchannel(uint8_t channel)  {

    return configuration.readParameter(CONF_MIDI_BLOCK, midiChannelSection, channel);

}

uint8_t MIDI::getParameter(uint8_t messageType, uint8_t parameterID)  {

    switch(messageType) {

        case midiFeatureConf:
        return getFeature(parameterID);
        break;

        case midiChannelConf:
        return getMIDIchannel(parameterID);
        break;

    }   return 0;

}


bool MIDI::setMIDIchannel(uint8_t channelID, uint8_t channelNumber)  {

    return configuration.writeParameter(CONF_MIDI_BLOCK, midiChannelSection, channelID, channelNumber);

}

bool MIDI::setFeature(uint8_t featureID, uint8_t newValue)  {

    if (!configuration.writeParameter(CONF_MIDI_BLOCK, midiFeatureSection, featureID, newValue))
        return false;

    if (featureID == midiFeatureRunningStatus)    {

        //tell hwMIDI object that we've changed this setting
        newValue ? hwMIDI.enableRunningStatus() : hwMIDI.disableRunningStatus();

    }   return true;

}

bool MIDI::setParameter(uint8_t messageType, uint8_t parameterID, uint8_t newValue) {

    switch(messageType) {

        case midiFeatureConf:
        return setFeature(parameterID, newValue);
        break;

        case midiChannelConf:
        return setMIDIchannel(parameterID, newValue);
        break;

    }   return false;

}

MIDI midi;
