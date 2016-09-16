/*
    OpenDeck MIDI platform firmware
    Copyright (C) 2015, 2016 Igor Petrovic

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "MIDI.h"
#include "../../sysex/SysEx.h"
#include "../../eeprom/Configuration.h"
#include "../../BitManipulation.h"
#include "../../board/Board.h"
#include "../../interface/Interface.h"

MIDI::MIDI()    {

    //default constructor

}

void MIDI::init() {

    uint8_t inChannel = configuration.readParameter(CONF_BLOCK_MIDI, midiChannelSection, inputChannel);
    hwMIDI.init(dinInterface);
    hwMIDI.init(usbInterface);
    hwMIDI.setInputChannel(inChannel);

}

void MIDI::checkInput()   {

    midiMessageType_t messageType;

    if (hwMIDI.read(usbInterface))   {   //new message on usb

        messageType = hwMIDI.getType(usbInterface);
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

        messageType = hwMIDI.getType(dinInterface);
        uint8_t data1 = hwMIDI.getData1(dinInterface);
        uint8_t data2 = hwMIDI.getData2(dinInterface);

        source = dinInterface;

        if (!configuration.readParameter(CONF_BLOCK_MIDI, midiFeatureSection, midiFeatureUSBconvert))  {

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
                    hwMIDI.sendNoteOff(data1, data2, configuration.readParameter(CONF_BLOCK_MIDI, midiChannelSection, inputChannel), usbInterface);
                    break;

                    case midiMessageNoteOn:
                    hwMIDI.sendNoteOn(data1, data2, configuration.readParameter(CONF_BLOCK_MIDI, midiChannelSection, inputChannel), usbInterface);
                    break;

                    case midiMessageControlChange:
                    hwMIDI.sendControlChange(data1, data2, configuration.readParameter(CONF_BLOCK_MIDI, midiChannelSection, inputChannel), usbInterface);
                    break;

                    case midiMessageProgramChange:
                    hwMIDI.sendProgramChange(data1, configuration.readParameter(CONF_BLOCK_MIDI, midiChannelSection, inputChannel), usbInterface);
                    break;

                    case midiMessageSystemExclusive:
                    hwMIDI.sendSysEx(hwMIDI.getSysExArrayLength(dinInterface), hwMIDI.getSysExArray(dinInterface), true, usbInterface);
                    break;

                    case midiMessageAfterTouchChannel:
                    hwMIDI.sendAfterTouch(data1, configuration.readParameter(CONF_BLOCK_MIDI, midiChannelSection, inputChannel), usbInterface);
                    break;

                    case midiMessageAfterTouchPoly:
                    hwMIDI.sendPolyPressure(data1, data2, configuration.readParameter(CONF_BLOCK_MIDI, midiChannelSection, inputChannel), usbInterface);
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

    uint8_t channel = configuration.readParameter(CONF_BLOCK_MIDI, midiChannelSection, noteChannel);

    switch (state) {

        case false:
        //button released
        if (configuration.readParameter(CONF_BLOCK_MIDI, midiFeatureSection, midiFeatureStandardNoteOff))   {

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

    uint8_t channel = configuration.readParameter(CONF_BLOCK_MIDI, midiChannelSection, programChangeChannel);
    hwMIDI.sendProgramChange(program, channel, usbInterface);
    hwMIDI.sendProgramChange(program, channel, dinInterface);

}

void MIDI::sendControlChange(uint8_t ccNumber, uint8_t ccValue) {

    uint8_t channel = configuration.readParameter(CONF_BLOCK_MIDI, midiChannelSection, CCchannel);
    hwMIDI.sendControlChange(ccNumber, ccValue, channel, usbInterface);
    hwMIDI.sendControlChange(ccNumber, ccValue, channel, dinInterface);

}

void MIDI::sendSysEx(uint8_t *sysExArray, uint8_t arraySize, bool arrayContainsBoundaries, bool usbSend, bool dinSend)   {

    if (usbSend) hwMIDI.sendSysEx(arraySize, sysExArray, arrayContainsBoundaries, usbInterface);
    if (dinSend) hwMIDI.sendSysEx(arraySize, sysExArray, arrayContainsBoundaries, dinInterface);

}

MIDI midi;
