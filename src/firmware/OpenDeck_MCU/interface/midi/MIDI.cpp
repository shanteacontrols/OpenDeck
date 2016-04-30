#include "MIDI.h"
#include "../../sysex/SysEx.h"
#include "../../eeprom/Configuration.h"
#include "../../BitManipulation.h"
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
    hwMIDI.init(inChannel, true, true, dinInterface);
    hwMIDI.init(inChannel, true, true, usbInterface);
    hwMIDI.setInputChannel(inChannel);

}

void MIDI::checkInput()   {

    if (hwMIDI.read(usbInterface))   {   //new message on usb

        midiMessageType_t messageType = hwMIDI.getType(usbInterface);
        uint8_t data1 = hwMIDI.getData1(usbInterface);
        uint8_t data2 = hwMIDI.getData2(usbInterface);
        source = usbInterface;

        switch(messageType) {

            case midiMessageSystemExclusive:
            sysEx.handleSysEx(hwMIDI.getSysExArray(usbInterface), hwMIDI.getSysExArrayLength(usbInterface));
            lastSysExMessageTime = rTimeMillis();
            break;

            case midiMessageNoteOff:
            case midiMessageNoteOn:
            //we're using received note data to control LEDs
            leds.noteToLEDstate(data1, data2);
            break;

            default:
            break;

        }

    }

    //check for incoming MIDI messages on USART
    if (hwMIDI.read(dinInterface))    {

        uint8_t messageType = hwMIDI.getType(dinInterface);
        uint8_t data1 = hwMIDI.getData1(dinInterface);
        uint8_t data2 = hwMIDI.getData2(dinInterface);

        source = dinInterface;

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
                    hwMIDI.sendNoteOff(data1, data2, getMIDIchannel(inputChannel), usbInterface);
                    break;

                    case midiMessageNoteOn:
                    hwMIDI.sendNoteOn(data1, data2, getMIDIchannel(inputChannel), usbInterface);
                    break;

                    case midiMessageControlChange:
                    hwMIDI.sendControlChange(data1, data2, getMIDIchannel(inputChannel), usbInterface);
                    break;

                    case midiMessageProgramChange:
                    hwMIDI.sendProgramChange(data1, getMIDIchannel(inputChannel), usbInterface);
                    break;

                    case midiMessageSystemExclusive:
                    hwMIDI.sendSysEx(hwMIDI.getSysExArrayLength(dinInterface), hwMIDI.getSysExArray(dinInterface), true, usbInterface);
                    break;

                    case midiMessageAfterTouchChannel:
                    hwMIDI.sendAfterTouch(data1, getMIDIchannel(inputChannel), usbInterface);
                    break;

                    case midiMessageAfterTouchPoly:
                    hwMIDI.sendPolyPressure(data1, data2, getMIDIchannel(inputChannel), usbInterface);
                    break;

                    default:
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

            hwMIDI.sendNoteOff(note, _velocity, channel, usbInterface);
            hwMIDI.sendNoteOff(note, _velocity, channel, dinInterface);

        } else {

            hwMIDI.sendNoteOn(note, _velocity, channel, usbInterface);
            hwMIDI.sendNoteOn(note, _velocity, channel, dinInterface);

        }
        break;

        case true:
        //button pressed
        hwMIDI.sendNoteOn(note, _velocity, channel, usbInterface);
        hwMIDI.sendNoteOn(note, _velocity, channel, dinInterface);
        break;

    }

}

void MIDI::sendProgramChange(uint8_t program)    {

    uint8_t channel = getMIDIchannel(programChangeChannel);
    hwMIDI.sendProgramChange(program, channel, usbInterface);
    hwMIDI.sendProgramChange(program, channel, dinInterface);

}

void MIDI::sendControlChange(uint8_t ccNumber, uint8_t ccValue) {

    uint8_t channel = getMIDIchannel(CCchannel);
    hwMIDI.sendControlChange(ccNumber, ccValue, channel, usbInterface);
    hwMIDI.sendControlChange(ccNumber, ccValue, channel, dinInterface);

}

void MIDI::sendSysEx(uint8_t *sysExArray, uint8_t arraySize)   {

    switch (source) {

        case dinInterface:
        hwMIDI.sendSysEx(arraySize, sysExArray, false, dinInterface);
        break;

        case usbInterface:
        hwMIDI.sendSysEx(arraySize, sysExArray, false, usbInterface);
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
