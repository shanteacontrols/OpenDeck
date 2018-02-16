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

#include "SysExConf.h"
#include "../interface/Interface.h"
#include "../Version.h"

//use enum for analog sysex sections due to 7/14 bit number conversion
typedef enum
{
    analogEnabled_sysExSection,
    analogInverted_sysExSection,
    analogType_sysExSection,
    analogMIDIidLSB_sysExSection,
    analogMIDIidMSB_sysExSection,
    analogLowerCClimitLSB_sysExSection,
    analogLowerCClimitMSB_sysExSection,
    analogUpperCClimitLSB_sysExSection,
    analogUpperCClimitMSB_sysExSection
} analogSysExSection_t;

SysExConfig::SysExConfig()
{
    
}

bool onCustom(uint8_t value)
{
    switch(value)
    {
        case FIRMWARE_VERSION_STRING:
        sysEx.addToResponse(SW_VERSION_MAJOR);
        sysEx.addToResponse(SW_VERSION_MINOR);
        sysEx.addToResponse(SW_VERSION_REVISION);
        return true;

        case HARDWARE_VERSION_STRING:
        sysEx.addToResponse(BOARD_ID);
        sysEx.addToResponse(HARDWARE_VERSION_MAJOR);
        sysEx.addToResponse(HARDWARE_VERSION_MINOR);
        sysEx.addToResponse(HARDWARE_VERSION_REVISION);
        return true;

        case FIRMWARE_HARDWARE_VERSION_STRING:
        sysEx.addToResponse(SW_VERSION_MAJOR);
        sysEx.addToResponse(SW_VERSION_MINOR);
        sysEx.addToResponse(SW_VERSION_REVISION);
        sysEx.addToResponse(BOARD_ID);
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
    encDec_14bit_t encDec_14bit;

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

        case DB_BLOCK_ANALOG:
        switch(section)
        {
            case analogMIDIidLSB_sysExSection:
            case analogMIDIidMSB_sysExSection:
            encDec_14bit.value = database.read(block, analogMIDIidSection, index);
            encDec_14bit.split14bit();

            if (section == analogMIDIidLSB_sysExSection)
                return encDec_14bit.low;
            else
                return encDec_14bit.high;
            break;

            case analogLowerCClimitLSB_sysExSection:
            case analogLowerCClimitMSB_sysExSection:
            encDec_14bit.value = database.read(block, analogCClowerLimitSection, index);
            encDec_14bit.split14bit();

            if (section == analogLowerCClimitLSB_sysExSection)
                return encDec_14bit.low;
            else
                return encDec_14bit.high;
            break;

            case analogUpperCClimitLSB_sysExSection:
            case analogUpperCClimitMSB_sysExSection:
            encDec_14bit.value = database.read(block, analogCCupperLimitSection, index);
            encDec_14bit.split14bit();

            if (section == analogUpperCClimitLSB_sysExSection)
                return encDec_14bit.low;
            else
                return encDec_14bit.high;
            break;

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
    encDec_14bit_t encDec_14bit;

    switch(block)
    {
        case DB_BLOCK_ANALOG:
        switch(section)
        {
            case analogMIDIidLSB_sysExSection:
            case analogMIDIidMSB_sysExSection:
            encDec_14bit.value = database.read(block, analogMIDIidSection, index);
            encDec_14bit.split14bit();

            if (section == analogMIDIidLSB_sysExSection)
                encDec_14bit.low = newValue;
            else
                encDec_14bit.high = newValue;

            encDec_14bit.mergeTo14bit();
            return database.update(block, analogMIDIidSection, index, encDec_14bit.value);
            break;

            case analogLowerCClimitLSB_sysExSection:
            case analogLowerCClimitMSB_sysExSection:
            encDec_14bit.value = database.read(block, analogCClowerLimitSection, index);
            encDec_14bit.split14bit();

            if (section == analogLowerCClimitLSB_sysExSection)
                encDec_14bit.low = newValue;
            else
                encDec_14bit.high = newValue;

            encDec_14bit.mergeTo14bit();
            return database.update(block, analogCClowerLimitSection, index, encDec_14bit.value);
            break;

            case analogUpperCClimitLSB_sysExSection:
            case analogUpperCClimitMSB_sysExSection:
            encDec_14bit.value = database.read(block, analogCCupperLimitSection, index);
            encDec_14bit.split14bit();

            if (section == analogUpperCClimitLSB_sysExSection)
                encDec_14bit.low = newValue;
            else
                encDec_14bit.high = newValue;

            encDec_14bit.mergeTo14bit();
            return database.update(block, analogCCupperLimitSection, index, encDec_14bit.value);
            break;

            case analogTypeSection:
            analog.debounceReset(index);
            return database.update(block, section, index, newValue);
            break;

            default:
            return database.update(block, section, index, newValue);
            break;
        }
        break;

        case DB_BLOCK_MIDI:
        if (section == midiFeatureSection)
        {
            switch(index)
            {
                case midiFeatureRunningStatus:
                midi.setRunningStatusState(newValue);
                break;

                case midiFeatureStandardNoteOff:
                newValue ? midi.setNoteOffMode(noteOffType_standardNoteOff) : midi.setNoteOffMode(noteOffType_noteOnZeroVel);
                break;

                case midiFeatureDinEnabled:
                midi.setDINMIDIstate(newValue);
                break;

                default:
                return false;
                break;
            }
        }

        return database.update(block, section, index, newValue);
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
            return database.update(block, section, index, newValue);
            break;

            case ledRGBenabledSection:
            //make sure to turn all three leds off before setting new state
            leds.setColor(board.getRGBaddress(board.getRGBID(index), rgb_R), colorOff);
            leds.setColor(board.getRGBaddress(board.getRGBID(index), rgb_G), colorOff);
            leds.setColor(board.getRGBaddress(board.getRGBID(index), rgb_B), colorOff);
            //write rgb enabled bit to three leds
            database.update(block, section, board.getRGBaddress(board.getRGBID(index), rgb_R), newValue);
            database.update(block, section, board.getRGBaddress(board.getRGBID(index), rgb_G), newValue);
            database.update(block, section, board.getRGBaddress(board.getRGBID(index), rgb_B), newValue);

            if (newValue)
            {
                //copy over note activation and local control settings from current led index to all three leds
                database.update(block, ledActivationNoteSection, board.getRGBaddress(board.getRGBID(index), rgb_R), database.read(block, ledActivationNoteSection, index));
                database.update(block, ledActivationNoteSection, board.getRGBaddress(board.getRGBID(index), rgb_G), database.read(block, ledActivationNoteSection, index));
                database.update(block, ledActivationNoteSection, board.getRGBaddress(board.getRGBID(index), rgb_B), database.read(block, ledActivationNoteSection, index));

                database.update(block, ledLocalControlSection, board.getRGBaddress(board.getRGBID(index), rgb_R), database.read(block, ledLocalControlSection, index));
                database.update(block, ledLocalControlSection, board.getRGBaddress(board.getRGBID(index), rgb_G), database.read(block, ledLocalControlSection, index));
                database.update(block, ledLocalControlSection, board.getRGBaddress(board.getRGBID(index), rgb_B), database.read(block, ledLocalControlSection, index));
            }
            break;

            case ledActivationNoteSection:
            case ledLocalControlSection:
            //first, find out if RGB led is enabled for this led index
            if (database.read(block, ledRGBenabledSection, index))
            {
                //rgb led enabled - copy these settings to all three leds
                database.update(block, section, board.getRGBaddress(board.getRGBID(index), rgb_R), newValue);
                database.update(block, section, board.getRGBaddress(board.getRGBID(index), rgb_G), newValue);
                database.update(block, section, board.getRGBaddress(board.getRGBID(index), rgb_B), newValue);
            }
            else
            {
                //apply to single led only
                database.update(block, section, index, newValue);
            }
            break;

            default:
            return database.update(block, section, index, newValue);
            break;
        }
        break;

        default:
        return database.update(block, section, index, newValue);
        break;
    }

    return true;
}

void writeSysEx(uint8_t sysExArray[], uint8_t arraysize)
{
    midi.sendSysEx(arraysize, sysExArray, true);
}

void SysExConfig::init()
{
    setHandleGet(onGet);
    setHandleSet(onSet);
    setHandleCustomRequest(onCustom);
    setHandleSysExWrite(writeSysEx);

    addCustomRequest(FIRMWARE_VERSION_STRING);
    addCustomRequest(HARDWARE_VERSION_STRING);
    addCustomRequest(FIRMWARE_HARDWARE_VERSION_STRING);
    addCustomRequest(MAX_COMPONENTS_STRING);
    addCustomRequest(REBOOT_APP_STRING);
    addCustomRequest(REBOOT_BTLDR_STRING);
    addCustomRequest(FACTORY_RESET_STRING);
}

SysExConfig sysEx;
