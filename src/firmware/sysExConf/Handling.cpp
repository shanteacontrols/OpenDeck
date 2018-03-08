/*
    OpenDeck MIDI platform firmware
    Copyright (C) 2015-2018 Igor Petrovic

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
    analogUpperCClimitMSB_sysExSection,
    analogMIDIchannel_sysExSection,
} analogSysExSection_t;

SysExConfig::SysExConfig()
{
    
}

bool onCustom(uint8_t value)
{
    bool requestValid = true;

    switch(value)
    {
        case FIRMWARE_VERSION_STRING:
        sysEx.addToResponse(SW_VERSION_MAJOR);
        sysEx.addToResponse(SW_VERSION_MINOR);
        sysEx.addToResponse(SW_VERSION_REVISION);
        break;

        case HARDWARE_VERSION_STRING:
        sysEx.addToResponse(BOARD_ID);
        sysEx.addToResponse(HARDWARE_VERSION_MAJOR);
        sysEx.addToResponse(HARDWARE_VERSION_MINOR);
        sysEx.addToResponse(HARDWARE_VERSION_REVISION);
        break;

        case FIRMWARE_HARDWARE_VERSION_STRING:
        sysEx.addToResponse(SW_VERSION_MAJOR);
        sysEx.addToResponse(SW_VERSION_MINOR);
        sysEx.addToResponse(SW_VERSION_REVISION);
        sysEx.addToResponse(BOARD_ID);
        sysEx.addToResponse(HARDWARE_VERSION_MAJOR);
        sysEx.addToResponse(HARDWARE_VERSION_MINOR);
        sysEx.addToResponse(HARDWARE_VERSION_REVISION);
        break;

        case MAX_COMPONENTS_STRING:
        sysEx.addToResponse(MAX_NUMBER_OF_BUTTONS+MAX_NUMBER_OF_ANALOG);
        sysEx.addToResponse(MAX_NUMBER_OF_ENCODERS);
        sysEx.addToResponse(MAX_NUMBER_OF_ANALOG);
        sysEx.addToResponse(MAX_NUMBER_OF_LEDS);
        break;

        case REBOOT_APP_STRING:
        leds.setAllOff();
        wait_ms(2500);
        board.reboot(rebootApp);
        return true;
        break;

        case REBOOT_BTLDR_STRING:
        leds.setAllOff();
        wait_ms(2500);
        board.reboot(rebootBtldr);
        return true;
        break;

        case FACTORY_RESET_STRING:
        leds.setAllOff();
        wait_ms(1500);
        database.factoryReset(initPartial);
        board.reboot(rebootApp);
        return true;
        break;

        default:
        requestValid = false;
        break;
    }

    if (requestValid)
    {
        #ifdef DISPLAY_SUPPORTED
        display.displayMIDIevent(displayEventIn, midiMessageSystemExclusive_display, 0, 0, 0);
        #endif
        return true;
    }

    return false;
}

sysExParameter_t onGet(uint8_t block, uint8_t section, uint16_t index)
{
    sysExParameter_t returnValue = 0;
    encDec_14bit_t encDec_14bit;

    switch(block)
    {
        case DB_BLOCK_LED:
        switch(section)
        {
            case ledColorSection:
            returnValue = leds.getColor(index);
            break;

            case ledBlinkSection:
            returnValue = leds.getBlinkState(index);
            break;

            case ledMIDIchannelSection:
            //channels start from 0 in db, start from 1 in sysex
            returnValue = database.read(block, section, index) + 1;
            break;

            default:
            returnValue = database.read(block, section, index);
            break;
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
                returnValue = encDec_14bit.low;
            else
                returnValue = encDec_14bit.high;
            break;

            case analogLowerCClimitLSB_sysExSection:
            case analogLowerCClimitMSB_sysExSection:
            encDec_14bit.value = database.read(block, analogCClowerLimitSection, index);
            encDec_14bit.split14bit();

            if (section == analogLowerCClimitLSB_sysExSection)
                returnValue = encDec_14bit.low;
            else
                returnValue = encDec_14bit.high;
            break;

            case analogUpperCClimitLSB_sysExSection:
            case analogUpperCClimitMSB_sysExSection:
            encDec_14bit.value = database.read(block, analogCCupperLimitSection, index);
            encDec_14bit.split14bit();

            if (section == analogUpperCClimitLSB_sysExSection)
                returnValue = encDec_14bit.low;
            else
                returnValue = encDec_14bit.high;
            break;

            case analogMIDIchannel_sysExSection:
            //channels start from 0 in db, start from 1 in sysex
            returnValue = database.read(block, analogMIDIchannelSection, index) + 1;
            break;

            default:
            returnValue = database.read(block, section, index);
            break;
        }
        break;

        case DB_BLOCK_BUTTON:
        //channels start from 0 in db, start from 1 in sysex
        if (section == buttonMIDIchannelSection)
            returnValue = database.read(block, section, index) + 1;
        else
            returnValue = database.read(block, section, index);
        break;

        case DB_BLOCK_ENCODER:
        //channels start from 0 in db, start from 1 in sysex
        if (section == encoderMIDIchannelSection)
            returnValue = database.read(block, section, index) + 1;
        else
            returnValue = database.read(block, section, index);
        break;

        case DB_BLOCK_DISPLAY:
        #ifdef DISPLAY_SUPPORTED
        returnValue = database.read(block, section, index);
        #else
        sysEx.setError(ERROR_NOT_SUPPORTED);
        #endif
        break;

        default:
        returnValue = database.read(block, section, index);
        break;
    }

    #ifdef DISPLAY_SUPPORTED
    display.displayMIDIevent(displayEventIn, midiMessageSystemExclusive_display, 0, 0, 0);
    #endif

    return returnValue;
}

bool onSet(uint8_t block, uint8_t section, uint16_t index, sysExParameter_t newValue)
{
    bool success = false;
    bool writeToDb = true;
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
            success = database.update(block, analogMIDIidSection, index, encDec_14bit.value);
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
            success = database.update(block, analogCClowerLimitSection, index, encDec_14bit.value);
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
            success = database.update(block, analogCCupperLimitSection, index, encDec_14bit.value);
            break;

            case analogType_sysExSection:
            analog.debounceReset(index);
            success = database.update(block, analogTypeSection, index, newValue);
            break;

            case analogMIDIchannel_sysExSection:
            //channels start from 0 in db, start from 1 in sysex
            success = database.update(block, analogMIDIchannelSection, index, newValue-1);
            break;

            default:
            success = database.update(block, section, index, newValue);
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
                success = true;
                break;

                case midiFeatureStandardNoteOff:
                newValue ? midi.setNoteOffMode(noteOffType_standardNoteOff) : midi.setNoteOffMode(noteOffType_noteOnZeroVel);
                success = true;
                break;

                case midiFeatureDinEnabled:
                midi.setDINMIDIstate(newValue);
                success = true;
                break;

                case midiFeatureThruEnabled:
                success = true;
                break;

                default:
                break;
            }
        }
        else if (section == midiThruSection)
        {
            switch(index)
            {
                case midiThruInterface:
                if ((newValue >=0) && (newValue < MIDI_THRU_INTERFACES))
                    success = true;
                break;

                default:
                break;
            }
        }

        if (success)
        {
            success = database.update(block, section, index, newValue);
        }
        break;

        case DB_BLOCK_LED:
        switch(section)
        {
            case ledColorSection:
            //no writing to database
            leds.setColor(index, (ledColor_t)newValue);
            success = true;
            writeToDb = false;
            break;

            case ledBlinkSection:
            //no writing to database
            leds.setBlinkState(index, newValue);
            success = true;
            writeToDb = false;
            break;

            case ledHardwareParameterSection:
            //this entire section needs specific value check since values differ
            //depending on index
            switch(index)
            {
                case ledHwParameterBlinkTime:
                if ((newValue >= BLINK_TIME_MIN) && (newValue <= BLINK_TIME_MAX))
                {
                    leds.setBlinkTime(newValue);
                    success = true;
                }
                break;

                case ledHwParameterFadeTime:
                if ((newValue >= FADE_TIME_MIN) && (newValue <= FADE_TIME_MAX))
                {
                    #ifdef BOARD_OPEN_DECK
                    leds.setFadeTime(newValue);
                    #endif
                    success = true;
                }
                break;

                case ledHwParameterStartUpRoutine:
                if ((newValue <= 1) && (newValue >= 0))
                    success = true;
                break;
            }

            //write to db if success is true and writing should take place
            if (success && writeToDb)
                success = database.update(block, section, index, newValue);
            break;

            case ledRGBenabledSection:
            //make sure to turn all three leds off before setting new state
            leds.setColor(board.getRGBaddress(board.getRGBID(index), rgb_R), colorOff);
            leds.setColor(board.getRGBaddress(board.getRGBID(index), rgb_G), colorOff);
            leds.setColor(board.getRGBaddress(board.getRGBID(index), rgb_B), colorOff);

            //write rgb enabled bit to three leds
            success = database.update(block, section, board.getRGBaddress(board.getRGBID(index), rgb_R), newValue);

            if (success)
            {
                success = database.update(block, section, board.getRGBaddress(board.getRGBID(index), rgb_G), newValue);

                if (success)
                {
                    success = database.update(block, section, board.getRGBaddress(board.getRGBID(index), rgb_B), newValue);
                }
            }

            if (newValue && success)
            {
                //copy over note activation and local control settings from current led index to all three leds
                success = database.update(block, ledActivationNoteSection, board.getRGBaddress(board.getRGBID(index), rgb_R), database.read(block, ledActivationNoteSection, index));

                if (success)
                {
                    success = database.update(block, ledActivationNoteSection, board.getRGBaddress(board.getRGBID(index), rgb_G), database.read(block, ledActivationNoteSection, index));

                    if (success)
                    {
                        success = database.update(block, ledActivationNoteSection, board.getRGBaddress(board.getRGBID(index), rgb_B), database.read(block, ledActivationNoteSection, index));

                        if (success)
                        {
                            success = database.update(block, ledLocalControlSection, board.getRGBaddress(board.getRGBID(index), rgb_R), database.read(block, ledLocalControlSection, index));

                            if (success)
                            {
                                success = database.update(block, ledLocalControlSection, board.getRGBaddress(board.getRGBID(index), rgb_G), database.read(block, ledLocalControlSection, index));

                                if (success)
                                {
                                    success = database.update(block, ledLocalControlSection, board.getRGBaddress(board.getRGBID(index), rgb_B), database.read(block, ledLocalControlSection, index));
                                }
                            }
                        }
                    }
                }
            }
            break;

            case ledActivationNoteSection:
            case ledLocalControlSection:
            //first, find out if RGB led is enabled for this led index
            if (database.read(block, ledRGBenabledSection, index))
            {
                //rgb led enabled - copy these settings to all three leds
                success = database.update(block, section, board.getRGBaddress(board.getRGBID(index), rgb_R), newValue);

                if (success)
                {
                    success = database.update(block, section, board.getRGBaddress(board.getRGBID(index), rgb_G), newValue);

                    if (success)
                    {
                        success = database.update(block, section, board.getRGBaddress(board.getRGBID(index), rgb_B), newValue);
                    }
                }
            }
            else
            {
                //apply to single led only
                success = database.update(block, section, index, newValue);
            }
            break;

            case ledMIDIchannelSection:
            //channels start from 0 in db, start from 1 in sysex
            success = database.update(block, section, index, newValue-1);
            break;

            default:
            success = database.update(block, section, index, newValue);
            break;
        }
        break;

        case DB_BLOCK_DISPLAY:
        #ifdef DISPLAY_SUPPORTED
        switch(section)
        {
            case displayHwSection:
            switch(index)
            {
                case displayHwController:
                if ((newValue <= DISPLAY_CONTROLLERS) && (newValue >= 0))
                {
                    display.init((displayController_t)newValue, (displayResolution_t)database.read(DB_BLOCK_DISPLAY, displayHwSection, displayHwResolution));
                    display.displayHome();
                    success = true;
                }
                break;

                case displayHwResolution:
                if ((newValue <= DISPLAY_RESOLUTIONS) && (newValue >= 0))
                {
                    display.init((displayController_t)database.read(DB_BLOCK_DISPLAY, displayHwSection, displayHwController), (displayResolution_t)newValue);
                    display.displayHome();
                    success = true;
                }
                break;

                default:
                break;
            }
            break;

            case displayFeaturesSection:
            switch(index)
            {
                case displayFeatureMIDIeventRetention:
                if ((newValue <= 1) && (newValue >= 0))
                {
                    display.setRetentionState(newValue);
                    success = true;
                }
                break;

                case displayFeatureMIDIeventTime:
                if ((newValue >= MIN_MESSAGE_RETENTION_TIME) && (newValue <= MAX_MESSAGE_RETENTION_TIME))
                {
                    display.setRetentionTime(newValue * 1000);
                    success = true;
                }
                break;

                default:
                break;
            }
            break;

            default:
            break;
        }
        #else
        sysEx.setError(ERROR_NOT_SUPPORTED);
        #endif

        if (success)
            success = database.update(block, section, index, newValue);
        break;

        case DB_BLOCK_BUTTON:
        if (section == buttonMIDIchannelSection)
        {
            //channels start from 0 in db, start from 1 in sysex
            success = database.update(block, section, index, newValue-1);
        }
        else
        {
            success = database.update(block, section, index, newValue);
        }
        break;

        case DB_BLOCK_ENCODER:
        if (section == encoderMIDIchannelSection)
        {
            //channels start from 0 in db, start from 1 in sysex
            success = database.update(block, section, index, newValue-1);
        }
        else
        {
            success = database.update(block, section, index, newValue);
        }
        break;

        default:
        //rest of the blocks - no custom handling
        success = database.update(block, section, index, newValue);
        break;
    }

    if (success)
    {
        #ifdef DISPLAY_SUPPORTED
        display.displayMIDIevent(displayEventIn, midiMessageSystemExclusive_display, 0, 0, 0);
        #endif

        return true;
    }

    return false;
}

void writeSysEx(uint8_t sysExArray[], uint8_t arraysize)
{
    midi.sendSysEx(arraysize, sysExArray, true);
}

void SysExConfig::init()
{
    createLayout();

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
