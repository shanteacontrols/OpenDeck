/*
    OpenDeck MIDI platform firmware
    Copyright (C) 2015-2017 Igor Petrovic

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

#include "OpenDeck.h"

uint32_t lastCinfoMsgTime[DB_BLOCKS];

uint32_t getLastCinfoMsgTime(uint8_t block)
{
    return lastCinfoMsgTime[block];
}

void updateCinfoTime(uint8_t block)
{
    lastCinfoMsgTime[block] = rTimeMs();
}

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

        case MAX_COMPONENTS_STRING:
        sysEx.addToResponse(MAX_NUMBER_OF_BUTTONS+MAX_NUMBER_OF_ANALOG);
        sysEx.addToResponse(MAX_NUMBER_OF_ENCODERS);
        sysEx.addToResponse(MAX_NUMBER_OF_ANALOG);
        sysEx.addToResponse(MAX_NUMBER_OF_LEDS);
        return true;

        case REBOOT_APP_STRING:
        leds.setAllOff();
        wait(2500);
        board.reboot(rebootApp);
        return true;

        case REBOOT_BTLDR_STRING:
        leds.setAllOff();
        wait(2500);
        board.reboot(rebootBtldr);
        return true;

        case FACTORY_RESET_STRING:
        leds.setAllOff();
        wait(1500);
        database.factoryReset(initPartial);
        board.reboot(rebootApp);
        return true;
    }

    return false;
}

sysExParameter_t onGet(uint8_t block, uint8_t section, uint16_t index)
{
    switch(block)
    {
        case DB_BLOCK_LED:
        switch(section)
        {
            case ledColorSection:
            return leds.getColor(index);

            case ledBlinkSection:
            return leds.getBlinkState(index);

            default:
            return database.read(block, section, index);
        }
        break;

        default:
        return database.read(block, section, index);
    }
}

bool onSet(uint8_t block, uint8_t section, uint16_t index, sysExParameter_t newValue)
{
    bool returnValue = true;
    //check this block manually
    if (block != DB_BLOCK_LED)
        returnValue = database.update(block, section, index, newValue);

    if (returnValue)
    {
        //special checks
        switch(block)
        {
            case DB_BLOCK_ANALOG:
            if (section == analogTypeSection)
                analog.debounceReset(index);
            break;

            case DB_BLOCK_MIDI:
            if (section == midiFeatureSection)
            {
                if (index == midiFeatureRunningStatus)
                    newValue ? midi.enableRunningStatus() : midi.disableRunningStatus();
                else if (index == midiFeatureStandardNoteOff)
                    newValue ? midi.setNoteOffMode(noteOffType_standardNoteOff) : midi.setNoteOffMode(noteOffType_noteOnZeroVel);
            }
            break;

            case DB_BLOCK_LED:
            switch(section)
            {
                case ledColorSection:
                //no writing to database
                leds.setColor(index, (ledColor_t)newValue);
                break;

                case ledBlinkSection:
                //no writing to database
                leds.setBlinkState(index, newValue);
                break;

                case ledHardwareParameterSection:
                //this entire section needs specific value check
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

                    case ledHwParameterStartUpRoutine:
                    if (newValue > 1)
                        return false;
                    break;
                }
                //values are ok - write
                database.update(block, section, index, newValue);
                break;

                case ledRGBenabledSection:
                //make sure to turn all three leds off before setting new state
                leds.setColor(board.getRGBaddress(index, rgb_R), colorOff);
                leds.setColor(board.getRGBaddress(index, rgb_G), colorOff);
                leds.setColor(board.getRGBaddress(index, rgb_B), colorOff);
                //write rgb enabled bit to three leds
                database.update(block, section, board.getRGBaddress(index, rgb_R), newValue);
                database.update(block, section, board.getRGBaddress(index, rgb_G), newValue);
                database.update(block, section, board.getRGBaddress(index, rgb_B), newValue);
                break;

                default:
                database.update(block, section, index, newValue);
                break;
            }
            break;

            default:
            break;
        }
    }

    return returnValue;
}

int main()
{
    globalInit();

    sysEx.setHandleGet(onGet);
    sysEx.setHandleSet(onSet);
    sysEx.setHandleCustomRequest(onCustom);

    sysEx.addCustomRequest(FIRMWARE_VERSION_STRING);
    sysEx.addCustomRequest(HARDWARE_VERSION_STRING);
    sysEx.addCustomRequest(MAX_COMPONENTS_STRING);
    sysEx.addCustomRequest(REBOOT_APP_STRING);
    sysEx.addCustomRequest(REBOOT_BTLDR_STRING);
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
                sysEx.handleMessage(midi.getSysExArray(usbInterface), midi.getSysExArrayLength(usbInterface));
                break;

                case midiMessageNoteOff:
                case midiMessageNoteOn:
                //we're using received note data to control LED color
                leds.noteToState(data1, data2);
                break;

                case midiMessageControlChange:
                //control change is used to control led blinking
                leds.ccToBlink(data1, data2);
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

            if (!database.read(DB_BLOCK_MIDI, midiFeatureSection, midiFeatureUSBconvert))
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

                    case midiMessageAfterTouchPoly:
                    midi.sendPolyPressure(data1, data2, inChannel);
                    break;

                    case midiMessageProgramChange:
                    midi.sendProgramChange(data1, inChannel);
                    break;

                    case midiMessageAfterTouchChannel:
                    midi.sendAfterTouch(data1, inChannel);
                    break;

                    case midiMessageSystemExclusive:
                    midi.sendSysEx(midi.getSysExArrayLength(dinInterface), midi.getSysExArray(dinInterface), true);
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
