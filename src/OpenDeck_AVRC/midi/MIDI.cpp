#include "MIDI.h"
#include "..\sysex/SysEx.h"
#include "..\eeprom/EEPROMsettings.h"
#include "..\sysex/ProtocolDefinitions.h"
#include "..\BitManipulation.h"
#include "..\midi\usb_midi\USBmidi.h"

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

subtype midiFeatureSubtype  = { MIDI_FEATURES, 0, 1 };
subtype midiChannelSubtype  = { NUMBER_OF_CHANNELS, 1, 16 };

const uint8_t midiSubtypeArray[] = {

    midiFeatureConf,
    midiChannelConf

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

    sendSysExCallback   = NULL;
    sendNoteCallback    = NULL;

}

void MIDI::init() {

    sysEx.addMessageType(SYS_EX_MT_MIDI, MIDI_SUBTYPES);

    for (int i=0; i<MIDI_SUBTYPES; i++)   {

        //define subtype messages
        sysEx.addMessageSubType(SYS_EX_MT_MIDI, midiSubtypeArray[i], midiParametersArray[i], midiNewParameterLowArray[i], midiNewParameterHighArray[i]);

    }

    #ifndef DEBUG_MODE
        uint8_t inChannel = getMIDIchannel(inputChannel);
        //read incoming MIDI messages on specified channel
        hwMIDI.begin(inChannel);
        usbMIDI.begin(inChannel);
    #endif

}

uint8_t MIDI::getParameter(uint8_t messageType, uint8_t parameterID)  {

    switch(messageType) {

        case midiFeatureConf:
        return getFeature(parameterID);
        break;

        case midiChannelConf:
        return getMIDIchannel(parameterID);
        break;

    }   return INVALID_VALUE;
}

bool MIDI::setParameter(uint8_t messageType, uint8_t parameterID, uint8_t newValue) {

    int16_t address;
    uint8_t featuresArray;

    switch(messageType) {

        case midiFeatureConf:
        address = EEPROM_FEATURES_MIDI;
        featuresArray = eeprom_read_byte((uint8_t*)address);
        if (newValue == RESET_VALUE)    bitWrite(featuresArray, parameterID, bitRead(pgm_read_byte(&(defConf[address])), parameterID));
        else                            bitWrite(featuresArray, parameterID, newValue);

        eeprom_update_byte((uint8_t*)address, featuresArray);
        return (featuresArray == eeprom_read_byte((uint8_t*)EEPROM_FEATURES_MIDI));
        break;

        case midiChannelConf:
        address = eepromMIDIchannelArray[parameterID];
        if (newValue == RESET_VALUE)
            eeprom_update_byte((uint8_t*)address, pgm_read_byte(&(defConf[address])));
        else eeprom_update_byte((uint8_t*)address, newValue);
        return (newValue == getMIDIchannel(parameterID));
        break;

    }   return false;

}

void MIDI::checkInput()   {

    #ifndef DEBUG_MODE
    if (usbMIDI.read())   {   //new message on usb

        uint8_t messageType = usbMIDI.getType();
        source = usbSource;

        switch(messageType) {

            case USBmidiSystemExclusive:
            sendSysExCallback(usbMIDI.getSysExArray(), usbMIDI.getData1());
            lastSysExMessageTime = rTimeMillis();
            break;

            case USBmidiNoteOff:
            case USBmidiNoteOn:
            sendNoteCallback(usbMIDI.getData1(), usbMIDI.getData2());
            break;

        }

    }

    //check for incoming MIDI messages on USART
    if (hwMIDI.read())    {

        uint8_t messageType = hwMIDI.getType();
        source = midiSource;

        switch(messageType) {

            case HwMIDInoteOff:
            case HwMIDInoteOn:
            sendNoteCallback(hwMIDI.getData1(), hwMIDI.getData2());
            break;

            case HwMIDIsystemExclusive:
            sendSysExCallback(hwMIDI.getSysExArray(), hwMIDI.getSysExArrayLength());
            break;

        }

    }

    //disable sysex config after inactivity
    if (rTimeMillis() - lastSysExMessageTime > CONFIG_TIMEOUT)
        sysEx.disableConf();
    #endif

}

bool MIDI::getFeature(uint8_t featureID)  {

    uint8_t features = eeprom_read_byte((uint8_t*)EEPROM_FEATURES_MIDI);
    return bitRead(features, featureID);

}

void MIDI::sendMIDInote(uint8_t buttonNote, bool buttonState, uint8_t _velocity)  {

    #ifndef DEBUG_MODE
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
    #endif

}

void MIDI::sendProgramChange(uint8_t program)    {

    #ifndef DEBUG_MODE
    uint8_t channel = getMIDIchannel(programChangeChannel);
    usbMIDI.sendProgramChange(program, channel);
    hwMIDI.sendProgramChange(program, channel);
    #endif

}

void MIDI::sendControlChange(uint8_t ccNumber, uint8_t ccValue) {

    #ifndef DEBUG_MODE
    uint8_t channel = getMIDIchannel(CCchannel);
    usbMIDI.sendControlChange(ccNumber, ccValue, channel);
    hwMIDI.sendControlChange(ccNumber, ccValue, channel);
    #endif
}

void MIDI::sendSysEx(uint8_t *sysExArray, uint8_t arraySize)   {

    #ifndef DEBUG_MODE
    uint8_t usbArraySize = arraySize+2;
    uint8_t usbSysExArray[usbArraySize];

    switch (source) {

        case midiSource:
        hwMIDI.sendSysEx(arraySize, sysExArray, false);
        break;

        case usbSource:
        //usbMIDI.sendSysEx needs start and stop byte (F0 and F7)

        //init array
        for (int i=0; i<usbArraySize; i++) usbSysExArray[i] = 0;

        //append sysex start byte
        usbSysExArray[0] = 0xF0;

        //copy array
        for (int i=0; i<arraySize; i++)
            usbSysExArray[i+1] = sysExArray[i];

        //append sysexstop byte
        usbSysExArray[arraySize+1] = 0xF7;

        //send modified array
        usbMIDI.sendSysEx(usbArraySize, usbSysExArray);
        break;

    }
    #endif

}

bool MIDI::setMIDIchannel(uint8_t channel, uint8_t channelNumber)  {

    eeprom_update_byte((uint8_t*)((int16_t)eepromMIDIchannelArray[channel]), channelNumber);
    return (channelNumber == eeprom_read_byte((uint8_t*)((int16_t)eepromMIDIchannelArray[channel])));

}

uint8_t MIDI::getMIDIchannel(uint8_t channel)  {

    return eeprom_read_byte((uint8_t*)((int16_t)eepromMIDIchannelArray[channel]));

}

void MIDI::setHandleSysEx(void(*fptr)(uint8_t sysExArray[], uint8_t arraySize))    {

    sendSysExCallback = fptr;

}

void MIDI::setHandleNote(void(*fptr)(uint8_t note, uint8_t noteVelocity))    {

    sendNoteCallback = fptr;

}

MIDI midi;