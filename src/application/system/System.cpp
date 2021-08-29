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
#include "Layout.h"
#include "bootloader/FwSelector/FwSelector.h"
#include "core/src/general/Timing.h"
#include "io/common/Common.h"
#include "core/src/general/Helpers.h"

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
    return _sysEx2DB_global[static_cast<uint8_t>(section)];
}

Database::Section::button_t System::dbSection(Section::button_t section)
{
    return _sysEx2DB_button[static_cast<uint8_t>(section)];
}

Database::Section::encoder_t System::dbSection(Section::encoder_t section)
{
    return _sysEx2DB_encoder[static_cast<uint8_t>(section)];
}

Database::Section::analog_t System::dbSection(Section::analog_t section)
{
    return _sysEx2DB_analog[static_cast<uint8_t>(section)];
}

Database::Section::leds_t System::dbSection(Section::leds_t section)
{
    return _sysEx2DB_leds[static_cast<uint8_t>(section)];
}

Database::Section::display_t System::dbSection(Section::display_t section)
{
    return _sysEx2DB_display[static_cast<uint8_t>(section)];
}

Database::Section::touchscreen_t System::dbSection(Section::touchscreen_t section)
{
    return _sysEx2DB_touchscreen[static_cast<uint8_t>(section)];
}

void System::handleSysEx(const uint8_t* array, size_t size)
{
    _sysExConf.handleMessage(array, size);

    if (_backupRequested)
    {
        backup();
        _backupRequested = false;
    }
}

uint8_t System::SysExDataHandler::customRequest(uint16_t request, CustomResponse& customResponse)
{
    uint8_t result = SysExConf::DataHandler::STATUS_OK;

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
        _system._database.factoryReset();
        break;

    case SYSEX_CR_REBOOT_APP:
        _system._hwa.reboot(FwSelector::fwType_t::application);
        break;

    case SYSEX_CR_REBOOT_BTLDR:
        _system._hwa.reboot(FwSelector::fwType_t::bootloader);
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
        customResponse.append(_system._database.getSupportedPresets());
    }
    break;

    case SYSEX_CR_BOOTLOADER_SUPPORT:
    {
        customResponse.append(1);
    }
    break;

    case SYSEX_CR_FULL_BACKUP:
    {
        //no response here, just set flag internally that backup needs to be done
        _system._backupRequested = true;
    }
    break;

    default:
    {
        result = SysExConf::DataHandler::STATUS_ERROR_RW;
    }
    break;
    }

    if (result == SysExConf::DataHandler::STATUS_OK)
        _system._display.displayMIDIevent(IO::Display::eventType_t::in, IO::Display::event_t::systemExclusive, 0, 0, 0);

    return result;
}

void System::DBhandlers::presetChange(uint8_t preset)
{
    _system._leds.setAllOff();

    if (_system._display.init(false))
        _system._display.displayMIDIevent(IO::Display::eventType_t::in, IO::Display::event_t::presetChange, preset, 0, 0);

    _system.forceComponentRefresh();
}

void System::DBhandlers::factoryResetStart()
{
    _system._leds.setAllOff();
}

void System::DBhandlers::factoryResetDone()
{
    // don't run this if database isn' t initialized yet to avoid mcu reset if
    //factory reset is needed initially
    if (_system._database.isInitialized())
        _system._hwa.reboot(FwSelector::fwType_t::application);
}

void System::DBhandlers::initialized()
{
    //nothing to do here
}

void System::TouchScreenHandlers::button(size_t index, bool state)
{
    _system._buttons.processButton(MAX_NUMBER_OF_BUTTONS + MAX_NUMBER_OF_ANALOG + index, state);
}

void System::TouchScreenHandlers::analog(size_t index, uint16_t value, uint16_t min, uint16_t max)
{
    value = core::misc::mapRange(static_cast<uint32_t>(value), static_cast<uint32_t>(min), static_cast<uint32_t>(max), static_cast<uint32_t>(0), static_cast<uint32_t>(_system._analog.adcType()));
    _system._analog.processReading(MAX_NUMBER_OF_ANALOG + index, value);
}

void System::TouchScreenHandlers::screenChange(size_t screenID)
{
    _system._leds.refresh();
}

bool System::init()
{
    _database.registerHandlers(_dbHandlers);
    _touchscreen.registerEventNotifier(_touchScreenHandlers);

    _cInfo.registerHandler([this](Database::block_t dbBlock, uint16_t componentID) {
        return sendCInfo(dbBlock, componentID);
    });

    _analog.registerButtonHandler([this](uint8_t analogIndex, bool value) {
        _buttons.processButton(analogIndex + MAX_NUMBER_OF_BUTTONS, value);
    });

    _hwa.registerOnUSBconnectionHandler([this]() {
        forceComponentRefresh();
    });

    if (!_hwa.init())
        return false;

    if (!_database.init())
        return false;

    _encoders.init();
    _display.init(true);
    _touchscreen.init(IO::Touchscreen::mode_t::normal);
    _leds.init();

    _sysExConf.setLayout(sysExLayout);
    _sysExConf.setupCustomRequests(customRequests);

    configureMIDI();

    return true;
}

bool System::sendCInfo(Database::block_t dbBlock, uint16_t componentID)
{
    if (_sysExConf.isConfigurationEnabled())
    {
        if ((core::timing::currentRunTimeMs() - _lastCinfoMsgTime[static_cast<uint8_t>(dbBlock)]) > COMPONENT_INFO_TIMEOUT)
        {
            uint16_t cInfoMessage[] = {
                SYSEX_CM_COMPONENT_ID,
                static_cast<uint16_t>(dbBlock),
                0,
                0
            };

            uint8_t high, low;
            SysExConf::split14bit(componentID, high, low);

            cInfoMessage[2] = high;
            cInfoMessage[3] = low;

            _sysExConf.sendCustomMessage(cInfoMessage, 4);
            _lastCinfoMsgTime[static_cast<uint8_t>(dbBlock)] = core::timing::currentRunTimeMs();
        }

        return true;
    }

    return false;
}

void System::configureMIDI()
{
    _midi.init(MIDI::interface_t::usb);
    _midi.setInputChannel(MIDI::MIDI_CHANNEL_OMNI);
    _midi.setNoteOffMode(isMIDIfeatureEnabled(midiFeature_t::standardNoteOff) ? MIDI::noteOffType_t::standardNoteOff : MIDI::noteOffType_t::noteOnZeroVel);
    _midi.setRunningStatusState(isMIDIfeatureEnabled(midiFeature_t::runningStatus));
    _midi.setChannelSendZeroStart(true);

#ifdef DIN_MIDI_SUPPORTED
    if (isMIDIfeatureEnabled(midiFeature_t::dinEnabled))
    {
        _midi.init(MIDI::interface_t::din);
        _midi.useRecursiveParsing(true);
    }
    else
    {
        _midi.deInit(MIDI::interface_t::din);
        _midi.useRecursiveParsing(false);
    }
#endif
}

bool System::isMIDIfeatureEnabled(midiFeature_t feature)
{
    return _database.read(Database::Section::global_t::midiFeatures, static_cast<size_t>(feature));
}

System::midiMergeType_t System::midiMergeType()
{
    return static_cast<midiMergeType_t>(_database.read(Database::Section::global_t::midiMerge, static_cast<size_t>(midiMerge_t::mergeType)));
}

void System::backup()
{
    uint8_t backupRequest[] = {
        0xF0,
        _sysExMID.id1,
        _sysExMID.id2,
        _sysExMID.id3,
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

    uint16_t presetChangeRequest[] = {
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

    uint8_t currentPreset = _database.getPreset();

    //make sure not to report any errors while performing backup
    _sysExConf.setSilentMode(true);

    //send internally created backup requests to sysex handler for all presets, blocks and presets
    for (uint8_t preset = 0; preset < _database.getSupportedPresets(); preset++)
    {
        _database.setPreset(preset);
        presetChangeRequest[presetChangeRequestPresetIndex] = preset;
        _sysExConf.sendCustomMessage(presetChangeRequest, presetChangeRequestSize, false);

        for (size_t block = 0; block < _sysExConf.blocks(); block++)
        {
            backupRequest[backupRequestBlockIndex] = block;

            for (size_t section = 0; section < _sysExConf.sections(block); section++)
            {
                if (
                    (block == static_cast<uint8_t>(System::block_t::leds)) &&
                    ((section == static_cast<uint8_t>(System::Section::leds_t::testColor)) ||
                     (section == static_cast<uint8_t>(System::Section::leds_t::testBlink))))
                    continue;    //testing sections, skip

                backupRequest[backupRequestSectionIndex] = section;
                _sysExConf.handleMessage(backupRequest, sizeof(backupRequest));
            }
        }
    }

    _database.setPreset(currentPreset);
    presetChangeRequest[presetChangeRequestPresetIndex] = currentPreset;
    _sysExConf.sendCustomMessage(presetChangeRequest, presetChangeRequestSize, false);

    //finally, send back full backup request to mark the end of sending
    uint16_t endMarker = SYSEX_CR_FULL_BACKUP;
    _sysExConf.sendCustomMessage(&endMarker, 1);
    _sysExConf.setSilentMode(false);
}

void System::SysExDataHandler::sendResponse(uint8_t* array, uint16_t size)
{
    //never send responses through DIN MIDI
    _system._midi.sendSysEx(size, array, true, MIDI::interface_t::usb);
}

void System::checkComponents()
{
    enum class componentCheck_t : uint8_t
    {
        buttons,
        encoders,
        analog,
        leds,
        display,
        touchscreen
    };

    static componentCheck_t componentCheck = componentCheck_t::buttons;

    switch (componentCheck)
    {
    case componentCheck_t::buttons:
        _buttons.update();
        componentCheck = componentCheck_t::encoders;
        break;

    case componentCheck_t::encoders:
        _encoders.update();
        componentCheck = componentCheck_t::analog;
        break;

    case componentCheck_t::analog:
        _analog.update();
        componentCheck = componentCheck_t::leds;
        break;

    case componentCheck_t::leds:
        _leds.checkBlinking();
        componentCheck = componentCheck_t::display;
        break;

    case componentCheck_t::display:
        _display.update();
        componentCheck = componentCheck_t::touchscreen;
        break;

    case componentCheck_t::touchscreen:
        _touchscreen.update();
        componentCheck = componentCheck_t::buttons;
        break;
    }
}

void System::checkMIDI()
{
    auto processMessage = [&](MIDI::interface_t interface) {
        //new message
        auto    messageType = _midi.getType(interface);
        uint8_t data1       = _midi.getData1(interface);
        uint8_t data2       = _midi.getData2(interface);
        uint8_t channel     = _midi.getChannel(interface);

        switch (messageType)
        {
        case MIDI::messageType_t::systemExclusive:
            //process sysex messages only from usb interface
            if (interface == MIDI::interface_t::usb)
                handleSysEx(_midi.getSysExArray(interface), _midi.getSysExArrayLength(interface));
            break;

        case MIDI::messageType_t::noteOn:
        case MIDI::messageType_t::noteOff:
        case MIDI::messageType_t::controlChange:
        case MIDI::messageType_t::programChange:
            if (messageType == MIDI::messageType_t::programChange)
                IO::Common::setProgram(channel, data1);

            if (messageType == MIDI::messageType_t::noteOff)
                data2 = 0;

            _leds.midiToState(messageType, data1, data2, channel, IO::LEDs::dataSource_t::external);

            switch (messageType)
            {
            case MIDI::messageType_t::noteOn:
                _display.displayMIDIevent(IO::Display::eventType_t::in, IO::Display::event_t::noteOn, data1, data2, channel + 1);
                break;

            case MIDI::messageType_t::noteOff:
                _display.displayMIDIevent(IO::Display::eventType_t::in, IO::Display::event_t::noteOff, data1, data2, channel + 1);
                break;

            case MIDI::messageType_t::controlChange:
                _display.displayMIDIevent(IO::Display::eventType_t::in, IO::Display::event_t::controlChange, data1, data2, channel + 1);

                for (int i = 0; i < MAX_NUMBER_OF_ENCODERS; i++)
                {
                    if (!_database.read(Database::Section::encoder_t::remoteSync, i))
                        continue;

                    if (_database.read(Database::Section::encoder_t::mode, i) != static_cast<int32_t>(IO::Encoders::type_t::tControlChange))
                        continue;

                    if (_database.read(Database::Section::encoder_t::midiChannel, i) != channel)
                        continue;

                    if (_database.read(Database::Section::encoder_t::midiID, i) != data1)
                        continue;

                    _encoders.setValue(i, data2);
                }
                break;

            case MIDI::messageType_t::programChange:
                _display.displayMIDIevent(IO::Display::eventType_t::in, IO::Display::event_t::programChange, data1, data2, channel + 1);
                _database.setPreset(data1);
                break;

            default:
                break;
            }
            break;

        case MIDI::messageType_t::sysRealTimeClock:
            _leds.checkBlinking(true);
            break;

        case MIDI::messageType_t::sysRealTimeStart:
            _leds.resetBlinking();
            _leds.checkBlinking(true);
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
        while (_midi.read(MIDI::interface_t::usb, MIDI::filterMode_t::fullDIN))
            processMessage(MIDI::interface_t::usb);
    }
    else
    {
        while (_midi.read(MIDI::interface_t::usb))
            processMessage(MIDI::interface_t::usb);
    }
#else
    while (_midi.read(MIDI::interface_t::usb))
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
                while (_midi.read(MIDI::interface_t::din, MIDI::filterMode_t::fullUSB))
                    ;
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
            while (_midi.read(MIDI::interface_t::din))
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

void System::forceComponentRefresh()
{
    _analog.update(true);
    _buttons.update(true);
}