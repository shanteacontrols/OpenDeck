#include "MIDI.h"
#include "..\sysex/SysEx.h"
#include "..\eeprom/EEPROMsettings.h"
#include "..\sysex/ProtocolDefinitions.h"
#include "..\BitManipulation.h"
#include "..\midi\usb_midi\USBmidi.h"
#include "..\LEDs.h"

typedef enum {

    noteChannel,
    programChangeChannel,
    CCchannel,
    HwMIDIpitchBendChannel,
    inputChannel,
    NUMBER_OF_CHANNELS

} channels;

typedef enum {

    midiFeatureConf,
    midiChannelConf,
    MIDI_SUBTYPES

} sysExMessageSubtypeMIDI;

typedef enum {

    midiFeatureStandardNoteOff,
    midiFeatureRunningStatus,
    midiFeatureUSBconvert,
    MIDI_FEATURES

} midiFeaturesParameters;

subtype midiFeatureSubtype  = { MIDI_FEATURES, 0, 1, EEPROM_FEATURES_MIDI, BIT_PARAMETER };
subtype midiChannelSubtype  = { NUMBER_OF_CHANNELS, 1, 16, EEPROM_MC_START, BYTE_PARAMETER };

const subtype *midiSubtypeArray[] = {

    &midiFeatureSubtype,
    &midiChannelSubtype

};

const uint8_t midiParametersArray[] = {

    midiFeatureSubtype.parameters,
    midiChannelSubtype.parameters

};

const uint8_t midiNewParameterLowArray[] = {

    midiFeatureSubtype.lowValue,
    midiChannelSubtype.lowValue

};

const uint8_t midiNewParameterHighArray[] = {

    midiFeatureSubtype.highValue,
    midiChannelSubtype.highValue

};

MIDI::MIDI()    {

    //default constructor

}

void MIDI::init() {

    sysEx.addMessageType(SYS_EX_MT_MIDI, MIDI_SUBTYPES);

    for (int i=0; i<MIDI_SUBTYPES; i++)   {

        //define subtype messages
        sysEx.addMessageSubType(SYS_EX_MT_MIDI, i, midiSubtypeArray[i]->parameters, midiSubtypeArray[i]->lowValue, midiSubtypeArray[i]->highValue);

    }

    uint8_t inChannel = getMIDIchannel(inputChannel);
    //read incoming MIDI messages on specified channel
    hwMIDI.begin(inChannel);
    usbMIDI.begin(inChannel);

}

uint8_t MIDI::getParameter(uint8_t messageType, uint8_t parameterID)  {

    return eepromSettings.readParameter(midiSubtypeArray[messageType]->eepromAddress, parameterID, midiSubtypeArray[messageType]->parameterType);

}

bool MIDI::setParameter(uint8_t messageType, uint8_t parameterID, uint8_t newValue) {

    if (!eepromSettings.writeParameter(midiSubtypeArray[messageType]->eepromAddress, parameterID, newValue, midiSubtypeArray[messageType]->parameterType))
        return false;

    switch(messageType) {

        case midiFeatureConf:
        if (parameterID == midiFeatureRunningStatus)    {

            //tell hwMIDI object that we've changed this setting
            newValue ? hwMIDI.enableRunningStatus() : hwMIDI.disableRunningStatus();

        }
        break;

        default:
        break;

    }   return true;

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

bool MIDI::getFeature(uint8_t featureID)  {

    return eepromSettings.readParameter(midiSubtypeArray[midiFeatureConf]->eepromAddress, featureID, midiSubtypeArray[midiFeatureConf]->parameterType);

}

void MIDI::sendMIDInote(uint8_t buttonNote, bool buttonState, uint8_t _velocity)  {

    uint8_t channel = getMIDIchannel(noteChannel);

    switch (buttonState) {

        case false:
        //button released
        if (getFeature(midiFeatureStandardNoteOff))   {

            usbMIDI.sendNoteOff(buttonNote, _velocity, channel);
            hwMIDI.sendNoteOff(buttonNote, _velocity, channel);

        } else {

            usbMIDI.sendNoteOn(buttonNote, _velocity, channel);
            hwMIDI.sendNoteOn(buttonNote, _velocity, channel);

        }
        break;

        case true:
        //button pressed
        usbMIDI.sendNoteOn(buttonNote, _velocity, channel);
        hwMIDI.sendNoteOn(buttonNote, _velocity, channel);
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

bool MIDI::setMIDIchannel(uint8_t channel, uint8_t channelNumber)  {

    return eepromSettings.writeParameter(midiSubtypeArray[midiChannelConf]->eepromAddress, channel, channelNumber, midiSubtypeArray[midiChannelConf]->parameterType);

}

uint8_t MIDI::getMIDIchannel(uint8_t channel)  {

    return eepromSettings.readParameter(midiSubtypeArray[midiChannelConf]->eepromAddress, channel, midiSubtypeArray[midiChannelConf]->parameterType);

}

MIDI midi;