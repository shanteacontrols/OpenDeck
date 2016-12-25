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

#include "init/Init.h"

#define FIRMWARE_VERSION_STRING     0x56
#define HARDWARE_VERSION_STRING     0x42
#define REBOOT_APP_STRING           0x7F
#define REBOOT_BTLDR_STRING         0x55
#define FACTORY_RESET_STRING        0x44

bool onCustom(uint8_t value)
{
    switch(value)
    {
        case FIRMWARE_VERSION_STRING:
        sysEx.addToResponse(getSWversion(version_major));
        sysEx.addToResponse(getSWversion(version_minor));
        sysEx.addToResponse(getSWversion(version_revision));
        return true;

        case HARDWARE_VERSION_STRING:
        sysEx.addToResponse(hardwareVersion.major);
        sysEx.addToResponse(hardwareVersion.minor);
        sysEx.addToResponse(hardwareVersion.revision);
        return true;

        case REBOOT_APP_STRING:
        leds.setFadeTime(1);
        leds.setAllOff();
        wait(1500);
        reboot();
        return true; //pointless, but whatever

        case REBOOT_BTLDR_STRING:
        leds.setFadeTime(1);
        leds.setAllOff();
        wait(1500);
        reboot();
        return true; //pointless, but whatever

        case FACTORY_RESET_STRING:
        leds.setFadeTime(1);
        leds.setAllOff();
        wait(1500);
        database.factoryReset(factoryReset_partial);
        reboot();
        return true;
    }

    return false;
}

sysExParameter_t onGet(uint8_t block, uint8_t section, uint16_t index)
{
    switch(block)
    {
        case CONF_BLOCK_LED:
        if (section == ledStateSection)
            return leds.getState(index);
        else
            return database.read(block, section, index);
        break;

        default:
        return database.read(block, section, index);
    }
}

bool onSet(uint8_t block, uint8_t section, uint16_t index, sysExParameter_t newValue)
{
    bool returnValue = true;
    //check this block manually
    if (block != CONF_BLOCK_LED)
        returnValue = database.update(block, section, index, newValue);

    if (returnValue)
    {
        //special checks
        switch(block)
        {
            case CONF_BLOCK_ANALOG:
            if (section == analogTypeSection)
                analog.debounceReset(index);
            break;

            case CONF_BLOCK_MIDI:
            if (section == midiFeatureSection)
            {
                if (index == midiFeatureRunningStatus)
                    newValue ? midi.enableRunningStatus() : midi.disableRunningStatus();
                else if (index == midiFeatureStandardNoteOff)
                    newValue ? midi.setNoteOffMode(noteOffType_standardNoteOff) : midi.setNoteOffMode(noteOffType_noteOnZeroVel);
            }
            break;

            case CONF_BLOCK_LED:
            if (section == ledStateSection)
            {
                leds.noteToState(index, newValue, true);
            }
            else
            {
                if (section == ledHardwareParameterSection)
                {
                    switch(index)
                    {
                        case ledHwParameterBlinkTime:
                        if ((newValue < BLINK_TIME_MIN) || (newValue > BLINK_TIME_MAX))
                            return false;
                        leds.setBlinkTime(newValue);
                        break;

                        case ledHwParameterFadeTime:
                        if ((newValue < FADE_TIME_MIN) || (newValue > FADE_TIME_MAX))
                            return false;
                        leds.setFadeTime(newValue);
                        break;

                        case ledHwParameterStartUpSwitchTime:
                        if ((newValue < START_UP_SWITCH_TIME_MIN) || (newValue > START_UP_SWITCH_TIME_MAX))
                            return false;
                        break;

                        case ledHwParameterStartUpRoutine:
                        if (newValue > NUMBER_OF_START_UP_ANIMATIONS)
                            return false;
                        break;
                    }
                }

                database.update(block, section, index, newValue);
            }
            break;

            default:
            break;
        }
    }

    return returnValue;
}

int main()  {

    globalInit();

    sysEx.setHandleGet(onGet);
    sysEx.setHandleSet(onSet);
    sysEx.setHandleCustomRequest(onCustom);

    sysEx.addCustomRequest(FIRMWARE_VERSION_STRING);
    sysEx.addCustomRequest(HARDWARE_VERSION_STRING);
    sysEx.addCustomRequest(REBOOT_APP_STRING);
    sysEx.addCustomRequest(FACTORY_RESET_STRING);

    while(1)
    {
        if (midi.read(usbInterface))
        {
            //new message on usb
            midiMessageType_t messageType = midi.getType(usbInterface);
            uint8_t data1 = midi.getData1(usbInterface);
            uint8_t data2 = midi.getData2(usbInterface);

            switch(messageType)
            {
                case midiMessageSystemExclusive:
                sysEx.handleSysEx(midi.getSysExArray(usbInterface), midi.getSysExArrayLength(usbInterface));
                break;

                case midiMessageNoteOff:
                case midiMessageNoteOn:
                //we're using received note data to control LEDs
                leds.noteToState(data1, data2);
                break;

                default:
                break;
            }
        }

        //check for incoming MIDI messages on USART
        if (midi.read(dinInterface))
        {
            midiMessageType_t messageType = midi.getType(dinInterface);
            uint8_t data1 = midi.getData1(dinInterface);
            uint8_t data2 = midi.getData2(dinInterface);

            if (!database.read(CONF_BLOCK_MIDI, midiFeatureSection, midiFeatureUSBconvert))
            {
                switch(messageType)
                {
                    case midiMessageNoteOff:
                    case midiMessageNoteOn:
                    leds.noteToState(data1, data2);
                    break;

                    default:
                    break;
                }
            }
            else
            {
                //dump everything from MIDI in to USB MIDI out
                uint8_t inChannel = midi.getChannel(dinInterface);
                //temporarily disable din midi out - send to usb only
                midi.disableDIN();

                switch(messageType)
                {
                    case midiMessageNoteOff:
                    midi.sendNoteOff(data1, data2, inChannel);
                    break;

                    case midiMessageNoteOn:
                    midi.sendNoteOn(data1, data2, inChannel);
                    break;

                    case midiMessageControlChange:
                    midi.sendControlChange(data1, data2, inChannel);
                    break;

                    case midiMessageProgramChange:
                    midi.sendProgramChange(data1, inChannel);
                    break;

                    case midiMessageSystemExclusive:
                    midi.sendSysEx(midi.getSysExArrayLength(dinInterface), midi.getSysExArray(dinInterface), true);
                    break;

                    case midiMessageAfterTouchChannel:
                    midi.sendAfterTouch(data1, inChannel);
                    break;

                    case midiMessageAfterTouchPoly:
                    midi.sendPolyPressure(data1, data2, inChannel);
                    break;

                    default:
                    break;
                }

                //enable din output again
                midi.enableDIN();
            }
        }

        buttons.update();
        analog.update();
        encoders.update();
    }
}