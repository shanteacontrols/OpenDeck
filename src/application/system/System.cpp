/*

Copyright 2015-2021 Igor Petrovic

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

#include "System.h"
#include "board/Board.h"
#include "Version.h"
#include "Layout.h"
#include "bootloader/FwSelector/FwSelector.h"
#include "core/src/general/Timing.h"
#include "io/common/Common.h"

Database::block_t System::dbBlock(uint8_t index)
{
    //sysex blocks and db blocks don't have 1/1 mapping

    auto sysExBlock = static_cast<block_t>(index);

    switch (sysExBlock)
    {
    case block_t::global:
        return Database::block_t::global;

    case block_t::buttons:
        return Database::block_t::buttons;

    case block_t::encoders:
        return Database::block_t::encoders;

    case block_t::analog:
        return Database::block_t::analog;

    case block_t::leds:
        return Database::block_t::leds;

    case block_t::display:
        return Database::block_t::display;

    case block_t::touchscreen:
        return Database::block_t::touchscreen;

    default:
        return Database::block_t::AMOUNT;
    }
}

Database::Section::global_t System::dbSection(Section::global_t section)
{
    return sysEx2DB_global[static_cast<uint8_t>(section)];
}

Database::Section::button_t System::dbSection(Section::button_t section)
{
    return sysEx2DB_button[static_cast<uint8_t>(section)];
}

Database::Section::encoder_t System::dbSection(Section::encoder_t section)
{
    return sysEx2DB_encoder[static_cast<uint8_t>(section)];
}

Database::Section::analog_t System::dbSection(Section::analog_t section)
{
    return sysEx2DB_analog[static_cast<uint8_t>(section)];
}

Database::Section::leds_t System::dbSection(Section::leds_t section)
{
    return sysEx2DB_leds[static_cast<uint8_t>(section)];
}

Database::Section::display_t System::dbSection(Section::display_t section)
{
    return sysEx2DB_display[static_cast<uint8_t>(section)];
}

Database::Section::touchscreen_t System::dbSection(Section::touchscreen_t section)
{
    return sysEx2DB_touchscreen[static_cast<uint8_t>(section)];
}

void System::handleSysEx(const uint8_t* array, size_t size)
{
    sysExConf.handleMessage(array, size);

    if (backupRequested)
    {
        backup();
        backupRequested = false;
    }
}

System::result_t System::SysExDataHandler::customRequest(size_t request, CustomResponse& customResponse)
{
    auto result = System::result_t::ok;

    auto appendSW = [&customResponse]() {
        customResponse.append(SW_VERSION_MAJOR);
        customResponse.append(SW_VERSION_MINOR);
        customResponse.append(SW_VERSION_REVISION);
    };

    auto appendHW = [&customResponse]() {
        customResponse.append((FW_UID >> 24) & static_cast<uint32_t>(0xFF));
        customResponse.append((FW_UID >> 16) & static_cast<uint32_t>(0xFF));
        customResponse.append((FW_UID >> 8) & static_cast<uint32_t>(0xFF));
        customResponse.append(FW_UID & static_cast<uint32_t>(0xFF));
    };

    switch (request)
    {
    case SYSEX_CR_FIRMWARE_VERSION:
    {
        appendSW();
    }
    break;

    case SYSEX_CR_HARDWARE_UID:
    {
        appendHW();
    }
    break;

    case SYSEX_CR_FIRMWARE_VERSION_HARDWARE_UID:
    {
        appendSW();
        appendHW();
    }
    break;

    case SYSEX_CR_FACTORY_RESET:
        system.database.factoryReset();
        break;

    case SYSEX_CR_REBOOT_APP:
        system.hwa.reboot(FwSelector::fwType_t::application);
        break;

    case SYSEX_CR_REBOOT_BTLDR:
        system.hwa.reboot(FwSelector::fwType_t::bootloader);
        break;

    case SYSEX_CR_REBOOT_CDC:
        system.hwa.reboot(FwSelector::fwType_t::cdc);
        break;

    case SYSEX_CR_MAX_COMPONENTS:
    {
        customResponse.append(MAX_NUMBER_OF_BUTTONS + MAX_NUMBER_OF_ANALOG + MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS);
        customResponse.append(MAX_NUMBER_OF_ENCODERS);
        customResponse.append(MAX_NUMBER_OF_ANALOG + MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS);
        customResponse.append(MAX_NUMBER_OF_LEDS + MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS);
        customResponse.append(MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS);
    }
    break;

    case SYSEX_CR_SUPPORTED_PRESETS:
    {
        customResponse.append(system.database.getSupportedPresets());
    }
    break;

    case SYSEX_CR_BOOTLOADER_SUPPORT:
    {
        customResponse.append(1);
    }
    break;

    case SYSEX_CR_ENABLE_PROCESSING:
    {
        system.processingEnabled = true;
    }
    break;

    case SYSEX_CR_DISABLE_PROCESSING:
    {
        system.processingEnabled = false;
    }
    break;

    case SYSEX_CR_FULL_BACKUP:
    {
        //no response here, just set flag internally that backup needs to be done
        system.backupRequested = true;
    }
    break;

    default:
    {
        result = System::result_t::error;
    }
    break;
    }

    if (result == System::result_t::ok)
        system.display.displayMIDIevent(IO::Display::eventType_t::in, IO::Display::event_t::systemExclusive, 0, 0, 0);

    return result;
}

void System::DBhandlers::presetChange(uint8_t preset)
{
    system.leds.setAllOff();

    if (system.display.init(false))
        system.display.displayMIDIevent(IO::Display::eventType_t::in, IO::Display::event_t::presetChange, preset, 0, 0);
}

void System::DBhandlers::factoryResetStart()
{
    system.leds.setAllOff();
}

void System::DBhandlers::factoryResetDone()
{
    // don't run this if database isn' t initialized yet to avoid mcu reset if
    //factory reset is needed initially
    if (system.database.isInitialized())
        system.hwa.reboot(FwSelector::fwType_t::application);
}

void System::DBhandlers::initialized()
{
    //nothing to do here
}

bool System::init()
{
    database.registerHandlers(dbHandlers);

    cInfo.registerHandler([this](Database::block_t dbBlock, SysExConf::sysExParameter_t componentID) {
        return sendCInfo(dbBlock, componentID);
    });

    analog.registerButtonHandler([this](uint8_t analogIndex, bool value) {
        buttons.processButton(analogIndex + MAX_NUMBER_OF_BUTTONS, value);
    });

    if (!hwa.init())
        return false;

    if (!database.init())
        return false;

    encoders.init();
    display.init(true);
    touchscreen.init();
    leds.init();

    sysExConf.setLayout(sysExLayout, static_cast<uint8_t>(block_t::AMOUNT));
    sysExConf.setupCustomRequests(customRequests, NUMBER_OF_CUSTOM_REQUESTS);

    configureMIDI();

    return true;
}

bool System::isProcessingEnabled()
{
    return processingEnabled;
}

bool System::sendCInfo(Database::block_t dbBlock, SysExConf::sysExParameter_t componentID)
{
    if (sysExConf.isConfigurationEnabled())
    {
        if ((core::timing::currentRunTimeMs() - lastCinfoMsgTime[static_cast<uint8_t>(dbBlock)]) > COMPONENT_INFO_TIMEOUT)
        {
            SysExConf::sysExParameter_t cInfoMessage[] = {
                SYSEX_CM_COMPONENT_ID,
                static_cast<SysExConf::sysExParameter_t>(dbBlock),
                0,
                0
            };

            uint8_t high, low;
            SysExConf::split14bit(componentID, high, low);

            cInfoMessage[2] = high;
            cInfoMessage[3] = low;

            sysExConf.sendCustomMessage(cInfoMessage, 4);
            lastCinfoMsgTime[static_cast<uint8_t>(dbBlock)] = core::timing::currentRunTimeMs();
        }

        return true;
    }

    return false;
}

void System::configureMIDI()
{
    midi.init();
    midi.enableUSBMIDI();
    midi.setInputChannel(MIDI_CHANNEL_OMNI);
    midi.setNoteOffMode(isMIDIfeatureEnabled(midiFeature_t::standardNoteOff) ? MIDI::noteOffType_t::standardNoteOff : MIDI::noteOffType_t::noteOnZeroVel);
    midi.setRunningStatusState(isMIDIfeatureEnabled(midiFeature_t::runningStatus));
    midi.setChannelSendZeroStart(true);

#ifdef DIN_MIDI_SUPPORTED
    if (isMIDIfeatureEnabled(midiFeature_t::dinEnabled))
    {
        midi.enableDINMIDI();
        bool mergeEnabled = isMIDIfeatureEnabled(midiFeature_t::mergeEnabled);

        //use recursive parsing when merging is active
        midi.useRecursiveParsing(mergeEnabled);

        //only configure master
        if (mergeEnabled)
        {
            configureMIDImerge(midiMergeType());
        }
        else
        {
            hwa.enableDINMIDI(false);
        }
    }
    else
    {
        hwa.disableDINMIDI();
    }
#endif
}

#ifdef DIN_MIDI_SUPPORTED
void System::configureMIDImerge(midiMergeType_t mergeType)
{
    switch (mergeType)
    {
    case midiMergeType_t::DINtoDIN:
    {
        //forward all incoming DIN MIDI data to DIN MIDI out
        hwa.enableDINMIDI(true);
    }
    break;

    case midiMergeType_t::DINtoUSB:
    {
        hwa.enableDINMIDI(false);
    }
    break;

    default:
        break;
    }
}
#endif

bool System::isMIDIfeatureEnabled(midiFeature_t feature)
{
    return database.read(Database::Section::global_t::midiFeatures, static_cast<size_t>(feature));
}

System::midiMergeType_t System::midiMergeType()
{
    return static_cast<midiMergeType_t>(database.read(Database::Section::global_t::midiMerge, static_cast<size_t>(midiMerge_t::mergeType)));
}

void System::backup()
{
    uint8_t backupRequest[] = {
        0xF0,
        sysExMID.id1,
        sysExMID.id2,
        sysExMID.id3,
        0x00,    //request
        0x7F,    //all message parts,
        static_cast<uint8_t>(SysExConf::wish_t::backup),
        static_cast<uint8_t>(SysExConf::amount_t::all),
        0x00,    //block - set later in the loop
        0x00,    //section - set later in the loop
        0x00,    //index MSB - unused but required
        0x00,    //index LSB - unused but required
        0x00,    //new value MSB - unused but required
        0x00,    //new value LSB - unused but required
        0xF7
    };

    SysExConf::sysExParameter_t presetChangeRequest[] = {
        static_cast<uint8_t>(SysExConf::wish_t::set),
        static_cast<uint8_t>(SysExConf::amount_t::single),
        static_cast<uint8_t>(System::block_t::global),
        static_cast<uint8_t>(System::Section::global_t::presets),
        0x00,    //index 0 (active preset) MSB
        0x00,    //index 0 (active preset) LSB
        0x00,    //preset value MSB - always 0
        0x00     //preset value LSB - set later in the loop
    };

    const uint8_t presetChangeRequestSize        = 8;
    const uint8_t presetChangeRequestPresetIndex = 7;
    const uint8_t backupRequestBlockIndex        = 8;
    const uint8_t backupRequestSectionIndex      = 9;

    uint8_t currentPreset = database.getPreset();

    //make sure not to report any errors while performing backup
    sysExConf.setSilentMode(true);

    //send internally created backup requests to sysex handler for all presets, blocks and presets
    for (uint8_t preset = 0; preset < database.getSupportedPresets(); preset++)
    {
        database.setPreset(preset);
        presetChangeRequest[presetChangeRequestPresetIndex] = preset;
        sysExConf.sendCustomMessage(presetChangeRequest, presetChangeRequestSize, false);

        for (size_t block = 0; block < static_cast<uint8_t>(System::block_t::AMOUNT); block++)
        {
            backupRequest[backupRequestBlockIndex] = block;

            for (size_t section = 0; section < sysExLayout[block].numberOfSections; section++)
            {
                if (
                    (block == static_cast<uint8_t>(System::block_t::leds)) &&
                    ((section == static_cast<uint8_t>(System::Section::leds_t::testColor)) ||
                     (section == static_cast<uint8_t>(System::Section::leds_t::testBlink))))
                    continue;    //testing sections, skip

                backupRequest[backupRequestSectionIndex] = section;
                sysExConf.handleMessage(backupRequest, sizeof(backupRequest));
            }
        }
    }

    database.setPreset(currentPreset);
    presetChangeRequest[presetChangeRequestPresetIndex] = currentPreset;
    sysExConf.sendCustomMessage(presetChangeRequest, presetChangeRequestSize, false);

    //finally, send back full backup request to mark the end of sending
    SysExConf::sysExParameter_t endMarker = SYSEX_CR_FULL_BACKUP;
    sysExConf.sendCustomMessage(&endMarker, 1);
    sysExConf.setSilentMode(false);
}

void System::SysExDataHandler::sendResponse(uint8_t* array, size_t size)
{
    //never send responses through DIN MIDI
    system.midi.disableDINMIDI();
    system.midi.sendSysEx(size, array, true);

    if (system.database.read(Database::Section::global_t::midiFeatures, static_cast<size_t>(System::midiFeature_t::dinEnabled)))
        system.midi.enableDINMIDI();
}

void System::checkComponents()
{
    if (isProcessingEnabled())
    {
        if (hwa.isDigitalInputAvailable())
        {
            buttons.update();
            encoders.update();
        }

        analog.update();

        leds.checkBlinking();
        display.update();

        touchscreen.update();
    }
}

void System::checkMIDI()
{
    auto processMessage = [&](MIDI::interface_t interface) {
        //new message
        auto    messageType = midi.getType(interface);
        uint8_t data1       = midi.getData1(interface);
        uint8_t data2       = midi.getData2(interface);
        uint8_t channel     = midi.getChannel(interface);

        switch (messageType)
        {
        case MIDI::messageType_t::systemExclusive:
            //process sysex messages only from usb interface
            if (interface == MIDI::interface_t::usb)
                handleSysEx(midi.getSysExArray(interface), midi.getSysExArrayLength(interface));
            break;

        case MIDI::messageType_t::noteOn:
        case MIDI::messageType_t::noteOff:
        case MIDI::messageType_t::controlChange:
        case MIDI::messageType_t::programChange:
            if (messageType == MIDI::messageType_t::programChange)
                IO::Common::setProgram(channel, data1);

            if (messageType == MIDI::messageType_t::noteOff)
                data2 = 0;

            leds.midiToState(messageType, data1, data2, channel, IO::LEDs::dataSource_t::external);

            switch (messageType)
            {
            case MIDI::messageType_t::noteOn:
                display.displayMIDIevent(IO::Display::eventType_t::in, IO::Display::event_t::noteOn, data1, data2, channel + 1);
                break;

            case MIDI::messageType_t::noteOff:
                display.displayMIDIevent(IO::Display::eventType_t::in, IO::Display::event_t::noteOff, data1, data2, channel + 1);
                break;

            case MIDI::messageType_t::controlChange:
                display.displayMIDIevent(IO::Display::eventType_t::in, IO::Display::event_t::controlChange, data1, data2, channel + 1);
                break;

            case MIDI::messageType_t::programChange:
                display.displayMIDIevent(IO::Display::eventType_t::in, IO::Display::event_t::programChange, data1, data2, channel + 1);
                break;

            default:
                break;
            }

            if (messageType == MIDI::messageType_t::programChange)
                database.setPreset(data1);

            if (messageType == MIDI::messageType_t::controlChange)
            {
                for (int i = 0; i < MAX_NUMBER_OF_ENCODERS; i++)
                {
                    if (!database.read(Database::Section::encoder_t::remoteSync, i))
                        continue;

                    if (database.read(Database::Section::encoder_t::mode, i) != static_cast<int32_t>(IO::Encoders::type_t::tControlChange))
                        continue;

                    if (database.read(Database::Section::encoder_t::midiChannel, i) != channel)
                        continue;

                    if (database.read(Database::Section::encoder_t::midiID, i) != data1)
                        continue;

                    encoders.setValue(i, data2);
                }
            }
            break;

        case MIDI::messageType_t::sysRealTimeClock:
            leds.checkBlinking(true);
            break;

        case MIDI::messageType_t::sysRealTimeStart:
            leds.resetBlinking();
            leds.checkBlinking(true);
            break;

        default:
            break;
        }
    };

#ifdef DIN_MIDI_SUPPORTED
    if (
        isMIDIfeatureEnabled(System::midiFeature_t::dinEnabled) &&
        isMIDIfeatureEnabled(System::midiFeature_t::passToDIN))
    {
        //pass the message to din
        if (midi.read(MIDI::interface_t::usb, MIDI::filterMode_t::fullDIN))
            processMessage(MIDI::interface_t::usb);
    }
    else
    {
        if (midi.read(MIDI::interface_t::usb))
            processMessage(MIDI::interface_t::usb);
    }
#else
    if (midi.read(MIDI::interface_t::usb))
        processMessage(MIDI::interface_t::usb);
#endif

#ifdef DIN_MIDI_SUPPORTED
    if (isMIDIfeatureEnabled(System::midiFeature_t::dinEnabled))
    {
        if (isMIDIfeatureEnabled(System::midiFeature_t::mergeEnabled))
        {
            auto mergeType = midiMergeType();

            switch (mergeType)
            {
            case System::midiMergeType_t::DINtoUSB:
                //dump everything from DIN MIDI in to USB MIDI out
                midi.read(MIDI::interface_t::din, MIDI::filterMode_t::fullUSB);
                break;

                // case System::midiMergeType_t::DINtoDIN:
                //loopback is automatically configured here
                // break;

            default:
                break;
            }
        }
        else
        {
            if (midi.read(MIDI::interface_t::din))
                processMessage(MIDI::interface_t::din);
        }
    }
#endif
}

void System::run()
{
    checkComponents();
    checkMIDI();
}