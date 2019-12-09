/*

Copyright 2015-2019 Igor Petrovic

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*/

#include "SysConfig.h"
#include "Version.h"
#include "Layout.h"
#include "core/src/general/Timing.h"
#include "common/OpenDeckMIDIformat/OpenDeckMIDIformat.h"
#include "board/Board.h"

bool SysConfig::onCustomRequest(size_t value)
{
    using namespace Board;

    bool retVal = true;

    auto appendSW = [this]() {
        addToResponse(SW_VERSION_MAJOR);
        addToResponse(SW_VERSION_MINOR);
        addToResponse(SW_VERSION_REVISION);
    };

    auto appendHW = [this]() {
        addToResponse((FW_UID >> 24) & static_cast<uint32_t>(0xFF));
        addToResponse((FW_UID >> 16) & static_cast<uint32_t>(0xFF));
        addToResponse((FW_UID >> 8) & static_cast<uint32_t>(0xFF));
        addToResponse(FW_UID & static_cast<uint32_t>(0xFF));
        addToResponse(HARDWARE_VERSION_MAJOR);
        addToResponse(HARDWARE_VERSION_MINOR);
        addToResponse(0);
    };

    switch (value)
    {
    case SYSEX_CR_FIRMWARE_VERSION:
        appendSW();
        break;

    case SYSEX_CR_HARDWARE_VERSION:
        appendHW();
        break;

    case SYSEX_CR_FIRMWARE_HARDWARE_VERSION:
        appendSW();
        appendHW();
        break;

    case SYSEX_CR_REBOOT_APP:
    case SYSEX_CR_FACTORY_RESET:
    case SYSEX_CR_REBOOT_BTLDR:
#ifdef LEDS_SUPPORTED
        leds.setAllOff();
        core::timing::waitMs(2500);
#endif

        if (value == SYSEX_CR_FACTORY_RESET)
            database.factoryReset(LESSDB::factoryResetType_t::partial);

        if (value == SYSEX_CR_REBOOT_BTLDR)
            Board::reboot(rebootType_t::rebootBtldr);
        else
            Board::reboot(rebootType_t::rebootApp);
        break;

    case SYSEX_CR_MAX_COMPONENTS:
        addToResponse(MAX_NUMBER_OF_BUTTONS + MAX_NUMBER_OF_ANALOG + MAX_TOUCHSCREEN_BUTTONS);
        addToResponse(MAX_NUMBER_OF_ENCODERS);
        addToResponse(MAX_NUMBER_OF_ANALOG);
        addToResponse(MAX_NUMBER_OF_LEDS);
        break;

    case SYSEX_CR_SUPPORTED_PRESETS:
        addToResponse(database.getSupportedPresets());
        break;

#ifdef DIN_MIDI_SUPPORTED
    case SYSEX_CR_DAISY_CHAIN:
        //received message from opendeck board in daisy chain configuration
        //check if this board is master
        if (static_cast<midiMergeType_t>(database.read(DB_BLOCK_GLOBAL, dbSection_global_midiMerge, midiMergeType)) == midiMergeODslave)
        {
            //slave
            //send sysex to next board in the chain on uart channel
            sendDaisyChainRequest();
            //now configure daisy-chain configuration
            configureMIDImerge(midiMergeODslave);
        }
        break;
#endif

    case SYSEX_CR_ENABLE_PROCESSING:
        processingEnabled = true;
        break;

    case SYSEX_CR_DISABLE_PROCESSING:
        processingEnabled = false;
        break;

    default:
        retVal = false;
        break;
    }

    if (retVal)
    {
#ifdef DISPLAY_SUPPORTED
        display.displayMIDIevent(Interface::Display::eventType_t::in, Interface::Display::event_t::systemExclusive, 0, 0, 0);
#endif
    }

    return retVal;
}

bool SysConfig::onGet(uint8_t block, uint8_t section, size_t index, SysExConf::sysExParameter_t& value)
{
    bool                 success = true;
    MIDI::encDec_14bit_t encDec_14bit;
    int32_t              readValue = 0;

    switch (block)
    {
    case SYSEX_BLOCK_GLOBAL:
        switch (section)
        {
        case sysExSection_global_midiFeature:
        case sysExSection_global_midiMerge:
            success = database.read(block, section, index, readValue);
            break;

        case sysExSection_global_presets:
            if (index == systemGlobal_ActivePreset)
                readValue = database.getPreset();
            else if (index == systemGlobal_presetPreserve)
                readValue = database.getPresetPreserveState();
            break;

        default:
            success = false;
            break;
        }
        break;

    case SYSEX_BLOCK_LEDS:
#ifdef LEDS_SUPPORTED
        switch (section)
        {
        case sysExSection_leds_testColor:
            readValue = static_cast<int32_t>(leds.getColor(index));
            break;

        case sysExSection_leds_testBlink:
            readValue = leds.getBlinkState(index);
            break;

        case sysExSection_leds_midiChannel:
            success = database.read(block, sysEx2DB_leds[section], index, readValue);

            //channels start from 0 in db, start from 1 in sysex
            if (success)
                readValue++;
            break;

        case sysExSection_leds_rgbEnable:
            success = database.read(block, sysEx2DB_leds[section], Board::io::getRGBID(index), readValue);
            break;

        default:
            success = database.read(block, sysEx2DB_leds[section], index, readValue);
            break;
        }
#else
        setError(SysExConf::status_t::errorNotSupported);
        success = false;
#endif
        break;

    case SYSEX_BLOCK_ANALOG:
        switch (section)
        {
        case sysExSection_analog_midiID_LSB:
        case sysExSection_analog_midiID_MSB:
        case sysExSection_analog_lowerLimit_LSB:
        case sysExSection_analog_lowerLimit_MSB:
        case sysExSection_analog_upperLimit_LSB:
        case sysExSection_analog_upperLimit_MSB:
            success = database.read(block, sysEx2DB_analog[section], index, readValue);

            if (success)
            {
                encDec_14bit.value = readValue;
                encDec_14bit.split14bit();

                switch (section)
                {
                case sysExSection_analog_midiID_LSB:
                case sysExSection_analog_lowerLimit_LSB:
                case sysExSection_analog_upperLimit_LSB:
                    readValue = encDec_14bit.low;
                    break;

                default:
                    readValue = encDec_14bit.high;
                    break;
                }
            }
            break;

        case sysExSection_analog_midiChannel:
            success = database.read(block, sysEx2DB_analog[section], index, readValue);

            //channels start from 0 in db, start from 1 in sysex
            if (success)
                readValue++;
            break;

        default:
            success = database.read(block, sysEx2DB_analog[section], index, readValue);
            break;
        }
        break;

    case SYSEX_BLOCK_BUTTONS:
        success = database.read(block, sysEx2DB_buttons[section], index, readValue);

        //channels start from 0 in db, start from 1 in sysex
        if ((section == sysExSection_buttons_midiChannel) && success)
            readValue++;
        break;

    case SYSEX_BLOCK_ENCODERS:
        success = database.read(block, sysEx2DB_encoders[section], index, readValue);

        if (success)
        {
            if ((section == sysExSection_encoders_midiID_lsb) || (section == sysExSection_encoders_midiID_msb))
            {
                encDec_14bit.value = readValue;
                encDec_14bit.split14bit();

                if (section == sysExSection_encoders_midiID_lsb)
                    readValue = encDec_14bit.low;
                else
                    readValue = encDec_14bit.high;
            }
            else if (section == sysExSection_encoders_midiChannel)
            {
                //channels start from 0 in db, start from 1 in sysex
                readValue++;
            }
        }
        break;

    case SYSEX_BLOCK_DISPLAY:
#ifdef DISPLAY_SUPPORTED
        success = database.read(block, sysEx2DB_display[section], index, readValue);
#else
        setError(SysExConf::status_t::errorNotSupported);
        success = false;
#endif
        break;

    default:
        success = database.read(block, section, index, readValue);
        break;
    }

#ifdef DISPLAY_SUPPORTED
    display.displayMIDIevent(Interface::Display::eventType_t::in, Interface::Display::event_t::systemExclusive, 0, 0, 0);
#endif

    if (success)
    {
        value = readValue;
        return true;
    }
    else
    {
        value = 0;
        return false;
    }
}

bool SysConfig::onSet(uint8_t block, uint8_t section, size_t index, SysExConf::sysExParameter_t newValue)
{
    bool                 success   = false;
    bool                 writeToDb = true;
    MIDI::encDec_14bit_t encDec_14bit;

    switch (block)
    {
    case SYSEX_BLOCK_GLOBAL:
        if (section == sysExSection_global_midiFeature)
        {
            switch (index)
            {
            case midiFeatureRunningStatus:
#ifndef DIN_MIDI_SUPPORTED
                setError(SysExConf::status_t::errorNotSupported);
#else
                midi.setRunningStatusState(newValue);
                success = true;
#endif
                break;

            case midiFeatureStandardNoteOff:
                newValue ? midi.setNoteOffMode(MIDI::noteOffType_t::standardNoteOff) : midi.setNoteOffMode(MIDI::noteOffType_t::noteOnZeroVel);
                success = true;
                break;

            case midiFeatureDinEnabled:
#ifndef DIN_MIDI_SUPPORTED
                setError(SysExConf::status_t::errorNotSupported);
#else
                newValue ? setupMIDIoverUART(UART_BAUDRATE_MIDI_STD, true, true) : Board::UART::deInit(UART_MIDI_CHANNEL);
                success = true;
#endif
                break;

            case midiFeatureMergeEnabled:
#ifndef DIN_MIDI_SUPPORTED
                setError(SysExConf::status_t::errorNotSupported);
#else
                if (database.read(DB_BLOCK_GLOBAL, dbSection_global_midiFeatures, midiFeatureDinEnabled) || !newValue)
                {
                    success = true;
                    //use recursive parsing when merging is active
                    midi.useRecursiveParsing(newValue);

                    //make sure everything is in correct state
                    if (!newValue)
                    {
                        setupMIDIoverUART(UART_BAUDRATE_MIDI_STD, true, true);
                    }
                    else
                    {
                        //restore active settings
                        midiMergeType_t mergeType = static_cast<midiMergeType_t>(database.read(DB_BLOCK_GLOBAL, dbSection_global_midiMerge, midiMergeType));

                        if (mergeType == midiMergeODslave)
                            mergeType = midiMergeODslaveInitial;

                        configureMIDImerge(mergeType);
                    }
                }
                else
                {
                    //invalid configuration - trying to enable merge functionality while din midi is disabled
                    setError(SysExConf::status_t::errorWrite);
                }
#endif
                break;

            default:
                break;
            }
        }
        else if (section == sysExSection_global_midiMerge)
        {
#ifndef DIN_MIDI_SUPPORTED
            setError(SysExConf::status_t::errorNotSupported);
#else
            switch (index)
            {
            case midiMergeType:
                if (database.read(DB_BLOCK_GLOBAL, dbSection_global_midiFeatures, midiFeatureDinEnabled) && database.read(DB_BLOCK_GLOBAL, dbSection_global_midiFeatures, midiFeatureMergeEnabled))
                {
                    if ((newValue >= 0) && (newValue < MIDI_MERGE_TYPES))
                    {
                        success = true;

                        if (static_cast<midiMergeType_t>(newValue) == midiMergeODslave)
                            newValue = midiMergeODslaveInitial;

                        configureMIDImerge(static_cast<midiMergeType_t>(newValue));
                    }
                    else
                    {
                        setError(SysExConf::status_t::errorNewValue);
                    }
                }
                else
                {
                    //invalid configuration
                    setError(SysExConf::status_t::errorWrite);
                }
                break;

            case midiMergeUSBchannel:
            case midiMergeDINchannel:
                //unused for now
                writeToDb = false;
                success   = true;
                break;

            default:
                break;
            }
#endif
        }
        else if (section == sysExSection_global_presets)
        {
            if (index == systemGlobal_ActivePreset)
            {
                if (newValue < database.getSupportedPresets())
                {
                    database.setPreset(newValue);
                    success   = true;
                    writeToDb = false;
                }
                else
                {
                    setError(SysExConf::status_t::errorNotSupported);
                }
            }
            else if (index == systemGlobal_presetPreserve)
            {
                if ((newValue <= 1) && (newValue >= 0))
                {
                    database.setPresetPreserveState(newValue);
                    success   = true;
                    writeToDb = false;
                }
            }
        }

        if (success && writeToDb)
            success = database.update(block, sysEx2DB_midi[section], index, newValue);
        break;

    case SYSEX_BLOCK_ANALOG:
        switch (section)
        {
        case sysExSection_analog_midiID_LSB:
        case sysExSection_analog_midiID_MSB:
        case sysExSection_analog_lowerLimit_LSB:
        case sysExSection_analog_lowerLimit_MSB:
        case sysExSection_analog_upperLimit_LSB:
        case sysExSection_analog_upperLimit_MSB:
            encDec_14bit.value = database.read(block, sysEx2DB_analog[section], index);
            encDec_14bit.split14bit();

            switch (section)
            {
            case sysExSection_analog_midiID_LSB:
            case sysExSection_analog_lowerLimit_LSB:
            case sysExSection_analog_upperLimit_LSB:
                encDec_14bit.low = newValue;
                break;

            default:
                encDec_14bit.high = newValue;
                break;
            }

            encDec_14bit.mergeTo14bit();
            success = database.update(block, sysEx2DB_analog[section], index, encDec_14bit.value);
            break;

        case sysExSection_analog_type:
            analog.debounceReset(index);
            success = database.update(block, sysEx2DB_analog[section], index, newValue);
            break;

        default:
            //channels start from 0 in db, start from 1 in sysex
            if (section == sysExSection_analog_midiChannel)
                newValue--;

            success = database.update(block, sysEx2DB_analog[section], index, newValue);
            break;
        }
        break;

    case SYSEX_BLOCK_LEDS:
#ifdef LEDS_SUPPORTED
        switch (section)
        {
        case sysExSection_leds_testColor:
            //no writing to database
            leds.setColor(index, static_cast<Interface::digital::output::LEDs::color_t>(newValue));
            success   = true;
            writeToDb = false;
            break;

        case sysExSection_leds_testBlink:
            //no writing to database
            leds.setBlinkState(index, newValue ? Interface::digital::output::LEDs::blinkSpeed_t::s500ms : Interface::digital::output::LEDs::blinkSpeed_t::noBlink);
            success   = true;
            writeToDb = false;
            break;

        case sysExSection_leds_global:
            switch (static_cast<Interface::digital::output::LEDs::setting_t>(index))
            {
            case Interface::digital::output::LEDs::setting_t::blinkWithMIDIclock:
                if ((newValue <= 1) && (newValue >= 0))
                {
                    success = true;
                    leds.setBlinkType(static_cast<Interface::digital::output::LEDs::blinkType_t>(newValue));
                }
                break;

            case Interface::digital::output::LEDs::setting_t::useStartupAnimation:
                if ((newValue <= 1) && (newValue >= 0))
                    success = true;
                break;

            case Interface::digital::output::LEDs::setting_t::fadeSpeed:
#ifdef LED_FADING
                if ((newValue >= FADE_TIME_MIN) && (newValue <= FADE_TIME_MAX))
                {
                    leds.setFadeTime(newValue);
                    success = true;
                }
#else
                setError(SysExConf::status_t::errorNotSupported);
#endif
                break;

            default:
                break;
            }

            //write to db if success is true and writing should take place
            if (success && writeToDb)
                success = database.update(block, sysEx2DB_leds[section], index, newValue);
            break;

        case sysExSection_leds_rgbEnable:
            //make sure to turn all three leds off before setting new state
            leds.setColor(Board::io::getRGBaddress(Board::io::getRGBID(index), Interface::digital::output::LEDs::rgbIndex_t::r), Interface::digital::output::LEDs::color_t::off);
            leds.setColor(Board::io::getRGBaddress(Board::io::getRGBID(index), Interface::digital::output::LEDs::rgbIndex_t::g), Interface::digital::output::LEDs::color_t::off);
            leds.setColor(Board::io::getRGBaddress(Board::io::getRGBID(index), Interface::digital::output::LEDs::rgbIndex_t::b), Interface::digital::output::LEDs::color_t::off);

            //write rgb enabled bit to led
            success = database.update(block, sysEx2DB_leds[section], Board::io::getRGBID(index), newValue);

            if (newValue && success)
            {
                //copy over note activation local control and midi channel settings to all three leds from the current led index

                for (int i = 0; i < 3; i++)
                {
                    success = database.update(block, sysEx2DB_leds[sysExSection_leds_activationID], Board::io::getRGBaddress(Board::io::getRGBID(index), static_cast<Interface::digital::output::LEDs::rgbIndex_t>(i)), database.read(block, sysEx2DB_leds[sysExSection_leds_activationID], index));

                    if (!success)
                        break;

                    success = database.update(block, sysEx2DB_leds[sysExSection_leds_controlType], Board::io::getRGBaddress(Board::io::getRGBID(index), static_cast<Interface::digital::output::LEDs::rgbIndex_t>(i)), database.read(block, sysEx2DB_leds[sysExSection_leds_controlType], index));

                    if (!success)
                        break;

                    success = database.update(block, sysEx2DB_leds[sysExSection_leds_midiChannel], Board::io::getRGBaddress(Board::io::getRGBID(index), static_cast<Interface::digital::output::LEDs::rgbIndex_t>(i)), database.read(block, sysEx2DB_leds[sysExSection_leds_midiChannel], index));

                    if (!success)
                        break;
                }
            }
            break;

        case sysExSection_leds_activationID:
        case sysExSection_leds_controlType:
        case sysExSection_leds_midiChannel:

            //channels start from 0 in db, start from 1 in sysex
            if (section == sysExSection_leds_midiChannel)
                newValue--;

            //first, find out if RGB led is enabled for this led index
            if (database.read(block, sysEx2DB_leds[sysExSection_leds_rgbEnable], Board::io::getRGBID(index)))
            {
                //rgb led enabled - copy these settings to all three leds
                for (int i = 0; i < 3; i++)
                {
                    success = database.update(block, sysEx2DB_leds[section], Board::io::getRGBaddress(Board::io::getRGBID(index), static_cast<Interface::digital::output::LEDs::rgbIndex_t>(i)), newValue);

                    if (!success)
                        break;
                }
            }
            else
            {
                //apply to single led only
                success = database.update(block, sysEx2DB_leds[section], index, newValue);
            }
            break;

        default:
            success = database.update(block, sysEx2DB_leds[section], index, newValue);
            break;
        }
#else
        setError(SysExConf::status_t::errorNotSupported);
        success = false;
#endif
        break;

    case SYSEX_BLOCK_DISPLAY:
#ifdef DISPLAY_SUPPORTED
        switch (section)
        {
        case sysExSection_display_features:
            switch (index)
            {
            case displayFeatureEnable:
                if ((newValue <= 1) && (newValue >= 0))
                {
                    success = true;

                    if (newValue)
                        display.init(static_cast<displayController_t>(database.read(DB_BLOCK_DISPLAY, dbSection_display_hw, displayHwController)), static_cast<displayResolution_t>(database.read(DB_BLOCK_DISPLAY, dbSection_display_hw, displayHwResolution)));
                    else    //init with invalid configuration
                        display.init(DISPLAY_CONTROLLERS, DISPLAY_RESOLUTIONS);
                }
                break;

            case displayFeatureWelcomeMsg:
            case displayFeatureVInfoMsg:
                if ((newValue <= 1) && (newValue >= 0))
                {
                    success = true;
                    //do nothing, these values are checked on startup
                }
                break;

            case displayFeatureMIDIeventRetention:
                if ((newValue <= 1) && (newValue >= 0))
                {
                    display.setRetentionState(newValue);
                    success = true;
                }
                break;

            case displayFeatureMIDInotesAlternate:
                if ((newValue <= 1) && (newValue >= 0))
                {
                    display.setAlternateNoteDisplay(newValue);
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

        case sysExSection_display_hw:
            switch (index)
            {
            case displayHwController:
                if ((newValue <= DISPLAY_CONTROLLERS) && (newValue >= 0))
                {
                    display.init(static_cast<displayController_t>(newValue), static_cast<displayResolution_t>(database.read(DB_BLOCK_DISPLAY, sysEx2DB_display[section], displayHwResolution)));
                    success = true;
                }
                break;

            case displayHwResolution:
                if ((newValue <= DISPLAY_RESOLUTIONS) && (newValue >= 0))
                {
                    display.init(static_cast<displayController_t>(database.read(DB_BLOCK_DISPLAY, sysEx2DB_display[section], displayHwController)), static_cast<displayResolution_t>(newValue));
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
        setError(SysExConf::status_t::errorNotSupported);
#endif

        if (success)
            success = database.update(block, sysEx2DB_display[section], index, newValue);
        break;

    case SYSEX_BLOCK_BUTTONS:
        //channels start from 0 in db, start from 1 in sysex
        if (section == sysExSection_buttons_midiChannel)
            newValue--;

        success = database.update(block, sysEx2DB_buttons[section], index, newValue);

        if (success)
        {
            if (
                (section == sysExSection_buttons_type) ||
                (section == sysExSection_buttons_midiMessage))
                buttons.reset(index);
        }
        break;

    case SYSEX_BLOCK_ENCODERS:
        //channels start from 0 in db, start from 1 in sysex
        if (section == sysExSection_encoders_midiChannel)
            newValue--;

        success = database.update(block, sysEx2DB_encoders[section], index, newValue);
        encoders.resetValue(index);
        break;

    default:
        //rest of the blocks - no custom handling
        success = database.update(block, section, index, newValue);
        break;
    }

    if (success)
    {
#ifdef DISPLAY_SUPPORTED
        display.displayMIDIevent(Interface::Display::eventType_t::in, Interface::Display::event_t::systemExclusive, 0, 0, 0);
#endif

        return true;
    }

    return false;
}

void SysConfig::onWrite(uint8_t sysExArray[], size_t arraysize)
{
    midi.sendSysEx(arraysize, sysExArray, true);
}

void SysConfig::init()
{
    setLayout(sysExLayout, SYSEX_BLOCKS);
    setupCustomRequests(customRequests, NUMBER_OF_CUSTOM_REQUESTS);
}

bool SysConfig::isProcessingEnabled()
{
    return processingEnabled;
}

#ifdef DIN_MIDI_SUPPORTED
void SysConfig::setupMIDIoverUART(uint32_t baudRate, bool initRX, bool initTX)
{
    Board::UART::init(UART_MIDI_CHANNEL, baudRate);

    if (initRX)
    {
        midi.handleUARTread([](uint8_t& data) {
            return Board::UART::read(UART_MIDI_CHANNEL, data);
        });
    }
    else
    {
        midi.handleUARTread(nullptr);
    }

    if (initTX)
    {
        midi.handleUARTwrite([](uint8_t data) {
            return Board::UART::write(UART_MIDI_CHANNEL, data);
        });
    }
    else
    {
        midi.handleUARTwrite(nullptr);
    }
}
#endif

void SysConfig::setupMIDIoverUSB()
{
#ifdef USB_MIDI_SUPPORTED
    midi.handleUSBread(Board::USB::readMIDI);
    midi.handleUSBwrite(Board::USB::writeMIDI);
#else
    //enable uart-to-usb link when usb isn't supported directly
    Board::UART::init(UART_USB_LINK_CHANNEL, UART_BAUDRATE_MIDI_OD);

    midi.handleUSBread([](MIDI::USBMIDIpacket_t& USBMIDIpacket) {
        OpenDeckMIDIformat::packetType_t odPacketType;

        if (OpenDeckMIDIformat::read(UART_USB_LINK_CHANNEL, USBMIDIpacket, odPacketType))
        {
            if (odPacketType == OpenDeckMIDIformat::packetType_t::midi)
                return true;
        }

        return false;
    });

    midi.handleUSBwrite([](MIDI::USBMIDIpacket_t& USBMIDIpacket) {
        return OpenDeckMIDIformat::write(UART_USB_LINK_CHANNEL, USBMIDIpacket, OpenDeckMIDIformat::packetType_t::midi);
    });
#endif
}

bool SysConfig::sendCInfo(dbBlockID_t dbBlock, SysExConf::sysExParameter_t componentID)
{
    if (isConfigurationEnabled())
    {
        if ((core::timing::currentRunTimeMs() - lastCinfoMsgTime[dbBlock]) > COMPONENT_INFO_TIMEOUT)
        {
            SysExConf::sysExParameter_t cInfoMessage[] = {
                SYSEX_CM_COMPONENT_ID,
                static_cast<SysExConf::sysExParameter_t>(dbBlock),
                static_cast<SysExConf::sysExParameter_t>(componentID)
            };

            sendCustomMessage(midi.usbMessage.sysexArray, cInfoMessage, 3);
            lastCinfoMsgTime[static_cast<uint8_t>(dbBlock)] = core::timing::currentRunTimeMs();
        }

        return true;
    }

    return false;
}

void SysConfig::configureMIDI()
{
    midi.setInputChannel(MIDI_CHANNEL_OMNI);
    midi.setNoteOffMode(static_cast<MIDI::noteOffType_t>(database.read(DB_BLOCK_GLOBAL, dbSection_global_midiFeatures, midiFeatureStandardNoteOff)));
    midi.setRunningStatusState(database.read(DB_BLOCK_GLOBAL, dbSection_global_midiFeatures, midiFeatureRunningStatus));
    midi.setChannelSendZeroStart(true);

    setupMIDIoverUSB();

#ifdef DIN_MIDI_SUPPORTED
    if (database.read(DB_BLOCK_GLOBAL, dbSection_global_midiFeatures, midiFeatureDinEnabled))
    {
        //use recursive parsing when merging is active
        midi.useRecursiveParsing(database.read(DB_BLOCK_GLOBAL, dbSection_global_midiFeatures, midiFeatureMergeEnabled));

        //only configure master
        if (database.read(DB_BLOCK_GLOBAL, dbSection_global_midiFeatures, midiFeatureMergeEnabled))
        {
            midiMergeType_t type = static_cast<midiMergeType_t>(database.read(DB_BLOCK_GLOBAL, dbSection_global_midiMerge, midiMergeType));

            if (type == midiMergeODslave)
                setupMIDIoverUART(UART_BAUDRATE_MIDI_OD, true, false);    //init only uart read interface for now
            else
                configureMIDImerge(type);
        }
        else
        {
            setupMIDIoverUART(UART_BAUDRATE_MIDI_STD, true, true);
        }
    }
    else
    {
        Board::UART::deInit(UART_MIDI_CHANNEL);
        midi.handleUARTread(nullptr);
        midi.handleUARTwrite(nullptr);
    }
#endif
}

#ifdef DIN_MIDI_SUPPORTED
void SysConfig::configureMIDImerge(midiMergeType_t mergeType)
{
    switch (mergeType)
    {
    case midiMergeODmaster:
        Board::UART::setLoopbackState(UART_MIDI_CHANNEL, false);
        Board::UART::init(UART_MIDI_CHANNEL, UART_BAUDRATE_MIDI_OD);
        //before enabling master configuration, send slave request to other boards
        sendDaisyChainRequest();

        midi.handleUSBread([](MIDI::USBMIDIpacket_t& USBMIDIpacket) {
            //dump everything from MIDI in to USB MIDI out
            MIDI::USBMIDIpacket_t            slavePacket;
            OpenDeckMIDIformat::packetType_t packetType;

            //use this function to forward all incoming data from other boards to usb
            if (OpenDeckMIDIformat::read(UART_MIDI_CHANNEL, slavePacket, packetType))
            {
                if (packetType == OpenDeckMIDIformat::packetType_t::midi)
                {
#ifdef USB_MIDI_SUPPORTED
                    Board::USB::writeMIDI(slavePacket);
#else
                    OpenDeckMIDIformat::write(UART_USB_LINK_CHANNEL, slavePacket, OpenDeckMIDIformat::packetType_t::midi);
#endif
                }
            }

//read usb midi data and forward it to uart in od format
#ifdef USB_MIDI_SUPPORTED
            if (Board::USB::readMIDI(USBMIDIpacket))
            {
                return OpenDeckMIDIformat::write(UART_MIDI_CHANNEL, USBMIDIpacket, OpenDeckMIDIformat::packetType_t::midiDaisyChain);
            }
#else
            if (OpenDeckMIDIformat::read(UART_USB_LINK_CHANNEL, USBMIDIpacket, packetType))
            {
                if (packetType == OpenDeckMIDIformat::packetType_t::midi)
                    return OpenDeckMIDIformat::write(UART_MIDI_CHANNEL, USBMIDIpacket, OpenDeckMIDIformat::packetType_t::midiDaisyChain);
            }
#endif

            return false;
        });

#ifdef USB_MIDI_SUPPORTED
        midi.handleUSBwrite(Board::USB::writeMIDI);
#else
        midi.handleUSBwrite([](MIDI::USBMIDIpacket_t& USBMIDIpacket) {
            return OpenDeckMIDIformat::write(UART_USB_LINK_CHANNEL, USBMIDIpacket, OpenDeckMIDIformat::packetType_t::midi);
        });
#endif
        //unused
        midi.handleUARTread(nullptr);
        midi.handleUARTwrite(nullptr);
        break;

    case midiMergeODslave:
        Board::UART::setLoopbackState(UART_MIDI_CHANNEL, false);
        Board::UART::init(UART_MIDI_CHANNEL, UART_BAUDRATE_MIDI_OD);
        //forward all incoming messages to other boards
        midi.handleUSBread([](MIDI::USBMIDIpacket_t& USBMIDIpacket) {
            OpenDeckMIDIformat::packetType_t packetType;

            if (OpenDeckMIDIformat::read(UART_MIDI_CHANNEL, USBMIDIpacket, packetType))
            {
                if (packetType != OpenDeckMIDIformat::packetType_t::internalCommand)
                    return OpenDeckMIDIformat::write(UART_MIDI_CHANNEL, USBMIDIpacket, packetType);
            }

            return false;
        });

        //write data to uart (opendeck format)
        midi.handleUSBwrite([](MIDI::USBMIDIpacket_t& USBMIDIpacket) {
            return OpenDeckMIDIformat::write(UART_MIDI_CHANNEL, USBMIDIpacket, OpenDeckMIDIformat::packetType_t::midi);
        });

        //no need for uart handlers
        midi.handleUARTread(nullptr);
        midi.handleUARTwrite(nullptr);
        break;

    case midiMergeODslaveInitial:
        //init only uart read interface for now
        setupMIDIoverUART(UART_BAUDRATE_MIDI_OD, true, false);
        break;

    case midiMergeDINtoDIN:
        //forward all incoming DIN MIDI data to DIN MIDI out
        //also send OpenDeck-generated traffic to DIN MIDI out
        setupMIDIoverUART(UART_BAUDRATE_MIDI_STD, false, true);
        Board::UART::setLoopbackState(UART_MIDI_CHANNEL, true);
        break;

    case midiMergeDINtoUSB:
        setupMIDIoverUSB();
        setupMIDIoverUART(UART_BAUDRATE_MIDI_STD, true, true);
        break;

    default:
        break;
    }
}

void SysConfig::sendDaisyChainRequest()
{
    //send the message directly in custom request sysex format
    const uint8_t daisyChainSysEx[8] = {
        0xF0,
        SYS_EX_CONF_M_ID_0,
        SYS_EX_CONF_M_ID_1,
        SYS_EX_CONF_M_ID_2,
        static_cast<uint8_t>(SysExConf::status_t::request),
        0x00,
        SYSEX_CR_DAISY_CHAIN,
        0xF7
    };

    for (int i = 0; i < 8; i++)
        Board::UART::write(UART_MIDI_CHANNEL, daisyChainSysEx[i]);

    while (!Board::UART::isTxEmpty(UART_MIDI_CHANNEL))
        ;
}

#endif