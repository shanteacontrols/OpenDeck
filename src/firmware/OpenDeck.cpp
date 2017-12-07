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

#include "interface/Interface.h"

SysEx sysEx;
MIDI midi;

void initSysEx()
{
    sysEx.addBlocks(DB_BLOCKS);

    sysExSection section;

    {
        //MIDI block

        //midi feature section
        section.numberOfParameters = MIDI_FEATURES;
        section.minValue = 0;
        section.maxValue = 1;

        sysEx.addSection(DB_BLOCK_MIDI, section);

        //midi channel section
        section.numberOfParameters = MIDI_CHANNELS;
        section.minValue = 1;
        section.maxValue = 16;

        sysEx.addSection(DB_BLOCK_MIDI, section);
    }

    {
        //button block

        //type section
        section.numberOfParameters = MAX_NUMBER_OF_BUTTONS+MAX_NUMBER_OF_ANALOG;
        section.minValue = 0;
        section.maxValue = BUTTON_TYPES-1;

        sysEx.addSection(DB_BLOCK_BUTTON, section);

        //midi message type section
        section.numberOfParameters = MAX_NUMBER_OF_BUTTONS+MAX_NUMBER_OF_ANALOG;
        section.minValue = 0;
        section.maxValue = BUTTON_MESSAGE_TYPES-1;

        sysEx.addSection(DB_BLOCK_BUTTON, section);

        //midi id section
        section.numberOfParameters = MAX_NUMBER_OF_BUTTONS+MAX_NUMBER_OF_ANALOG;
        section.minValue = 0;
        section.maxValue = 127;

        sysEx.addSection(DB_BLOCK_BUTTON, section);
    }

    {
        //encoder block

        //encoder enabled section
        section.numberOfParameters = MAX_NUMBER_OF_ENCODERS;
        section.minValue = 0;
        section.maxValue = 1;

        sysEx.addSection(DB_BLOCK_ENCODER, section);

        //encoder inverted section
        section.numberOfParameters = MAX_NUMBER_OF_ENCODERS;
        section.minValue = 0;
        section.maxValue = 1;

        sysEx.addSection(DB_BLOCK_ENCODER, section);

        //encoding mode section
        section.numberOfParameters = MAX_NUMBER_OF_ENCODERS;
        section.minValue = 0;
        section.maxValue = ENCODING_MODES-1;

        sysEx.addSection(DB_BLOCK_ENCODER, section);

        //midi id section
        section.numberOfParameters = MAX_NUMBER_OF_ENCODERS;
        section.minValue = 0;
        section.maxValue = 127;

        sysEx.addSection(DB_BLOCK_ENCODER, section);
    }

    {
        //analog block

        //analog enabled section
        section.numberOfParameters = MAX_NUMBER_OF_ANALOG;
        section.minValue = 0;
        section.maxValue = 1;

        sysEx.addSection(DB_BLOCK_ANALOG, section);

        //analog inverted section
        section.numberOfParameters = MAX_NUMBER_OF_ANALOG;
        section.minValue = 0;
        section.maxValue = 1;

        sysEx.addSection(DB_BLOCK_ANALOG, section);

        //analog type section
        section.numberOfParameters = MAX_NUMBER_OF_ANALOG;
        section.minValue = 0;
        section.maxValue = ANALOG_TYPES-1;

        sysEx.addSection(DB_BLOCK_ANALOG, section);

        //midi id section
        section.numberOfParameters = MAX_NUMBER_OF_ANALOG;
        section.minValue = 0;
        section.maxValue = 127;

        sysEx.addSection(DB_BLOCK_ANALOG, section);

        //lower cc limit
        section.numberOfParameters = MAX_NUMBER_OF_ANALOG;
        section.minValue = 0;
        section.maxValue = 127;

        sysEx.addSection(DB_BLOCK_ANALOG, section);

        //upper cc limit
        section.numberOfParameters = MAX_NUMBER_OF_ANALOG;
        section.minValue = 0;
        section.maxValue = 127;

        sysEx.addSection(DB_BLOCK_ANALOG, section);
    }

    {
        //led block

        //hardware parameters section
        section.numberOfParameters = LED_HARDWARE_PARAMETERS;
        section.minValue = 0;
        section.maxValue = 0;

        sysEx.addSection(DB_BLOCK_LED, section);

        //activation note section
        section.numberOfParameters = MAX_NUMBER_OF_LEDS;
        section.minValue = 0;
        section.maxValue = 127;

        sysEx.addSection(DB_BLOCK_LED, section);

        //rgb enabled section
        section.numberOfParameters = MAX_NUMBER_OF_LEDS;
        section.minValue = 0;
        section.maxValue = 1;

        sysEx.addSection(DB_BLOCK_LED, section);

        //local led control enabled section
        section.numberOfParameters = MAX_NUMBER_OF_LEDS;
        section.minValue = 0;
        section.maxValue = 1;

        sysEx.addSection(DB_BLOCK_LED, section);

        //led color section
        section.numberOfParameters = MAX_NUMBER_OF_LEDS;
        section.minValue = 0;
        section.maxValue = LED_COLORS-1;

        sysEx.addSection(DB_BLOCK_LED, section);

        //led blink section
        section.numberOfParameters = MAX_NUMBER_OF_LEDS;
        section.minValue = 0;
        section.maxValue = 1;

        sysEx.addSection(DB_BLOCK_LED, section);
    }
}

void globalInit()
{
    #if defined(BOARD_OPEN_DECK) || defined(BOARD_A_LEO)
    midi.setUSBMIDIstate(true);
    #endif

    #if defined(BOARD_OPEN_DECK) || defined(BOARD_A_MEGA)
    midi.setDINMIDIstate(true);
    #endif

    midi.setOneByteParseDINstate(true);

    database.init();
    board.init();

    midi.setInputChannel(database.read(DB_BLOCK_MIDI, midiChannelSection, inputChannel));
    initSysEx();

    //enable global interrupts
    sei();

    leds.init();
}

bool onCustom(uint8_t value)
{
    switch(value)
    {
        case FIRMWARE_VERSION_STRING:
        sysEx.addToResponse(1);
        sysEx.addToResponse(0);
        sysEx.addToResponse(0);
        return true;

        case HARDWARE_VERSION_STRING:
        sysEx.addToResponse(HARDWARE_VERSION_MAJOR);
        sysEx.addToResponse(HARDWARE_VERSION_MINOR);
        sysEx.addToResponse(HARDWARE_VERSION_REVISION);
        return true;

        case MAX_COMPONENTS_STRING:
        sysEx.addToResponse(MAX_NUMBER_OF_BUTTONS+MAX_NUMBER_OF_ANALOG);
        sysEx.addToResponse(MAX_NUMBER_OF_ENCODERS);
        sysEx.addToResponse(MAX_NUMBER_OF_ANALOG);
        sysEx.addToResponse(MAX_NUMBER_OF_LEDS);
        return true;

        case REBOOT_APP_STRING:
        leds.setAllOff();
        wait_ms(2500);
        board.reboot(rebootApp);
        return true;

        case REBOOT_BTLDR_STRING:
        leds.setAllOff();
        wait_ms(2500);
        board.reboot(rebootBtldr);
        return true;

        case FACTORY_RESET_STRING:
        leds.setAllOff();
        wait_ms(1500);
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
                    midi.setRunningStatusState(newValue);
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
                    #ifdef BOARD_OPEN_DECK
                    leds.setFadeTime(newValue);
                    #endif
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

void writeSysEx(uint8_t sysExArray[], uint8_t arraysize)
{
    midi.sendSysEx(arraysize, sysExArray, true);
}

int main()
{
    globalInit();

    sysEx.setHandleGet(onGet);
    sysEx.setHandleSet(onSet);
    sysEx.setHandleCustomRequest(onCustom);
    sysEx.setHandleSysExWrite(writeSysEx);

    sysEx.addCustomRequest(FIRMWARE_VERSION_STRING);
    sysEx.addCustomRequest(HARDWARE_VERSION_STRING);
    sysEx.addCustomRequest(MAX_COMPONENTS_STRING);
    sysEx.addCustomRequest(REBOOT_APP_STRING);
    sysEx.addCustomRequest(REBOOT_BTLDR_STRING);
    sysEx.addCustomRequest(FACTORY_RESET_STRING);

    while(1)
    {
        #if defined(BOARD_A_LEO) || defined(BOARD_OPEN_DECK)
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

                case midiMessageNoteOn:
                //we're using received note data to control LED color
                leds.noteToState(data1, data2);
                break;

                case midiMessageNoteOff:
                //always turn led off when note off is received
                leds.noteToState(data1, 0);
                break;

                case midiMessageControlChange:
                //control change is used to control led blinking
                leds.ccToBlink(data1, data2);
                break;

                default:
                break;
            }
        }
        #endif

        #ifndef BOARD_A_MEGA
        if (midi.getDINMIDIstate())
        {
            //check for incoming MIDI messages on USART
            if (!database.read(DB_BLOCK_MIDI, midiFeatureSection, midiFeatureUSBconvert))
            {
                midi.read(dinInterface);
                midiMessageType_t messageType = midi.getType(dinInterface);
                uint8_t data1 = midi.getData1(dinInterface);
                uint8_t data2 = midi.getData2(dinInterface);

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
                midi.read(dinInterface, THRU_FULL_USB);
            }
        }
        #endif

        #ifdef BOARD_A_MEGA
        if (midi.read(dinInterface))
        {
            midiMessageType_t messageType = midi.getType(dinInterface);
            uint8_t data1 = midi.getData1(dinInterface);
            uint8_t data2 = midi.getData2(dinInterface);

            switch(messageType)
            {
                case midiMessageSystemExclusive:
                sysEx.handleMessage(midi.getSysExArray(dinInterface), midi.getSysExArrayLength(dinInterface));
                break;

                case midiMessageNoteOn:
                //we're using received note data to control LED color
                leds.noteToState(data1, data2);
                break;

                case midiMessageNoteOff:
                //always turn led off when note off is received
                leds.noteToState(data1, 0);
                break;

                case midiMessageControlChange:
                //control change is used to control led blinking
                leds.ccToBlink(data1, data2);
                break;

                default:
                break;
            }
        }
        #endif

        digitalInput.update();
        analog.update();
        leds.update();
    }
}
