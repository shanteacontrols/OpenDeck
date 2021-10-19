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

void System::SysExDataHandler::sendResponse(uint8_t* array, uint16_t size)
{
    // never send responses through DIN MIDI
    _system._midi.sendSysEx(size, array, true, MIDI::interface_t::usb);
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
    {
        _system._database.factoryReset();
    }
    break;

    case SYSEX_CR_REBOOT_APP:
    {
        _system._hwa.reboot(FwSelector::fwType_t::application);
    }
    break;

    case SYSEX_CR_REBOOT_BTLDR:
    {
        _system._hwa.reboot(FwSelector::fwType_t::bootloader);
    }
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
        // no response here, just set flag internally that backup needs to be done
        _system._backupRestoreState = backupRestoreState_t::backup;
    }
    break;

    case SYSEX_CR_RESTORE_START:
    {
        _system._backupRestoreState = backupRestoreState_t::restore;
    }
    break;

    case SYSEX_CR_RESTORE_END:
    {
        _system._backupRestoreState = backupRestoreState_t::none;
    }
    break;

    default:
    {
        result = SysExConf::DataHandler::STATUS_ERROR_RW;
    }
    break;
    }

    return result;
}

void System::DBhandlers::presetChange(uint8_t preset)
{
    _system._leds.setAllOff();

    if (_system._backupRestoreState == backupRestoreState_t::none)
    {
        _system._display.setPreset(preset);
        _system._scheduler.registerTask({ [this]() { _system.forceComponentRefresh(); }, FORCED_VALUE_RESEND_DELAY });
    }
}

void System::DBhandlers::factoryResetStart()
{
    _system._leds.setAllOff();
}

void System::DBhandlers::factoryResetDone()
{
    // don't run this if database isn't initialized yet to avoid mcu reset if
    // factory reset is needed initially
    if (_system._database.isInitialized())
        _system._hwa.reboot(FwSelector::fwType_t::application);
}

void System::DBhandlers::initialized()
{
    // nothing to do here
}

void System::TouchScreenHandlers::button(size_t index, bool state)
{
    Util::MessageDispatcher::message_t dispatchMessage;

    dispatchMessage.componentIndex = index;
    dispatchMessage.midiValue      = state;

    // mark this as forwarding message type - further action/processing is required
    _system._dispatcher.notify(Util::MessageDispatcher::messageSource_t::touchscreenButton,
                               dispatchMessage,
                               Util::MessageDispatcher::listenType_t::forward);
}

void System::TouchScreenHandlers::analog(size_t index, uint16_t value, uint16_t min, uint16_t max)
{
    Util::MessageDispatcher::message_t dispatchMessage;

    dispatchMessage.componentIndex = index;
    dispatchMessage.midiValue      = core::misc::mapRange(static_cast<uint32_t>(value),
                                                     static_cast<uint32_t>(min),
                                                     static_cast<uint32_t>(max),
                                                     static_cast<uint32_t>(0),
                                                     static_cast<uint32_t>(ADC_RESOLUTION));

    // mark this as forwarding message type - further action/processing is required
    _system._dispatcher.notify(Util::MessageDispatcher::messageSource_t::touchscreenAnalog,
                               dispatchMessage,
                               Util::MessageDispatcher::listenType_t::forward);
}

void System::TouchScreenHandlers::screenChange(size_t screenID)
{
    _system._leds.refresh();
}

void System::configureMIDI()
{
    _midi.init(MIDI::interface_t::usb);
    _midi.setInputChannel(MIDI::MIDI_CHANNEL_OMNI);
    _midi.setNoteOffMode(isMIDIfeatureEnabled(midiFeature_t::standardNoteOff) ? MIDI::noteOffType_t::standardNoteOff : MIDI::noteOffType_t::noteOnZeroVel);
    _midi.setRunningStatusState(isMIDIfeatureEnabled(midiFeature_t::runningStatus));
    _midi.setChannelSendZeroStart(true);

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
}

bool System::init()
{
    _cInfo.registerHandler([this](size_t group, size_t index) {
        if (_sysExConf.isConfigurationEnabled())
        {
            uint16_t cInfoMessage[] = {
                SYSEX_CM_COMPONENT_ID,
                static_cast<uint16_t>(group),
                0,
                0
            };

            uint8_t high, low;
            SysExConf::split14bit(index, high, low);

            cInfoMessage[2] = high;
            cInfoMessage[3] = low;

            _sysExConf.sendCustomMessage(cInfoMessage, 4);
        }
    });

    _hwa.registerOnUSBconnectionHandler([this]() {
        _scheduler.registerTask({ [this]() { forceComponentRefresh(); }, FORCED_VALUE_RESEND_DELAY });
    });

    if (!_hwa.init())
        return false;

    if (!_database.init())
        return false;

    _database.registerHandlers(_dbHandlers);
    _touchscreen.registerEventNotifier(_touchScreenHandlers);

    _display.init(true);
    _touchscreen.init(IO::Touchscreen::mode_t::normal);
    _leds.init();

    _sysExConf.setLayout(sysExLayout);
    _sysExConf.setupCustomRequests(customRequests);

    configureMIDI();

    uniqueID_t uniqueID;
    _hwa.uniqueID(uniqueID);

    uint32_t dmxSerialNr = uniqueID[0];
    dmxSerialNr <<= 8;
    dmxSerialNr |= uniqueID[1];
    dmxSerialNr <<= 8;
    dmxSerialNr |= uniqueID[2];
    dmxSerialNr <<= 8;
    dmxSerialNr |= uniqueID[3];

    _dmx.setWidgetInfo({ dmxSerialNr,
                         ESTA_ID,
                         0x00,
                         { SW_VERSION_MAJOR,
                           SW_VERSION_MINOR,
                           SW_VERSION_REVISION },
                         "Shantea Controls",
                         BOARD_STRING });

    if (_database.read(Database::Section::global_t::dmx, dmxSetting_t::enabled))
        _dmx.init();

    return true;
}

void System::backup()
{
    uint8_t backupRequest[] = {
        0xF0,
        _sysExMID.id1,
        _sysExMID.id2,
        _sysExMID.id3,
        0x00,    // request
        0x7F,    // all message parts,
        static_cast<uint8_t>(SysExConf::wish_t::backup),
        static_cast<uint8_t>(SysExConf::amount_t::all),
        0x00,    // block - set later in the loop
        0x00,    // section - set later in the loop
        0x00,    // index MSB - unused but required
        0x00,    // index LSB - unused but required
        0x00,    // new value MSB - unused but required
        0x00,    // new value LSB - unused but required
        0xF7
    };

    uint16_t presetChangeRequest[] = {
        static_cast<uint8_t>(SysExConf::wish_t::set),
        static_cast<uint8_t>(SysExConf::amount_t::single),
        static_cast<uint8_t>(System::block_t::global),
        static_cast<uint8_t>(System::Section::global_t::presets),
        0x00,    // index 0 (active preset) MSB
        0x00,    // index 0 (active preset) LSB
        0x00,    // preset value MSB - always 0
        0x00     // preset value LSB - set later in the loop
    };

    const uint8_t presetChangeRequestSize        = 8;
    const uint8_t presetChangeRequestPresetIndex = 7;
    const uint8_t backupRequestBlockIndex        = 8;
    const uint8_t backupRequestSectionIndex      = 9;

    uint8_t currentPreset = _database.getPreset();

    // make sure not to report any errors while performing backup
    _sysExConf.setSilentMode(true);

    // first message sent as an response should be restore start marker
    // this is used to indicate that restore procedure is in progress
    uint16_t restoreMarker = SYSEX_CR_RESTORE_START;
    _sysExConf.sendCustomMessage(&restoreMarker, 1, false);

    // send internally created backup requests to sysex handler for all presets, blocks and presets
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
                    continue;    // testing sections, skip

                backupRequest[backupRequestSectionIndex] = section;
                _sysExConf.handleMessage(backupRequest, sizeof(backupRequest));
            }
        }
    }

    _database.setPreset(currentPreset);
    presetChangeRequest[presetChangeRequestPresetIndex] = currentPreset;
    _sysExConf.sendCustomMessage(presetChangeRequest, presetChangeRequestSize, false);

    // mark the end of restore procedure
    restoreMarker = SYSEX_CR_RESTORE_END;
    _sysExConf.sendCustomMessage(&restoreMarker, 1, false);

    // finally, send back full backup request to mark the end of sending
    uint16_t endMarker = SYSEX_CR_FULL_BACKUP;
    _sysExConf.sendCustomMessage(&endMarker, 1);
    _sysExConf.setSilentMode(false);

    _backupRestoreState = backupRestoreState_t::none;
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
    {
        _buttons.update();
        componentCheck = componentCheck_t::encoders;
    }
    break;

    case componentCheck_t::encoders:
    {
        _encoders.update();
        componentCheck = componentCheck_t::analog;
    }
    break;

    case componentCheck_t::analog:
    {
        _analog.update();
        componentCheck = componentCheck_t::leds;
    }
    break;

    case componentCheck_t::leds:
    {
        _leds.update();
        componentCheck = componentCheck_t::display;
    }
    break;

    case componentCheck_t::display:
    {
        _display.update();
        componentCheck = componentCheck_t::touchscreen;
    }
    break;

    case componentCheck_t::touchscreen:
    {
        _touchscreen.update();
        componentCheck = componentCheck_t::buttons;
    }
    break;
    }
}

void System::checkMIDI()
{
    auto processMessage = [&](MIDI::interface_t interface) {
        Util::MessageDispatcher::message_t dispatchMessage;

        dispatchMessage.componentIndex = 0;
        dispatchMessage.midiChannel    = _midi.getChannel(interface);
        dispatchMessage.midiIndex      = _midi.getData1(interface);
        dispatchMessage.midiValue      = _midi.getData2(interface);
        dispatchMessage.message        = _midi.getType(interface);

        switch (dispatchMessage.message)
        {
        case MIDI::messageType_t::systemExclusive:
        {
            // process sysex messages only from usb interface
            if (interface == MIDI::interface_t::usb)
            {
                _sysExConf.handleMessage(_midi.getSysExArray(interface), _midi.getSysExArrayLength(interface));

                if (_backupRestoreState == backupRestoreState_t::backup)
                    backup();
            }
        }
        break;

        case MIDI::messageType_t::programChange:
        {
            IO::Common::setProgram(dispatchMessage.midiChannel, dispatchMessage.midiIndex);
            _database.setPreset(dispatchMessage.midiIndex);
        }
        break;

        case MIDI::messageType_t::noteOff:
        {
            dispatchMessage.midiValue = 0;
        }
        break;

        default:
            break;
        }

        _dispatcher.notify(Util::MessageDispatcher::messageSource_t::midiIn,
                           dispatchMessage,
                           Util::MessageDispatcher::listenType_t::nonFwd);
    };

    if (
        isMIDIfeatureEnabled(System::midiFeature_t::dinEnabled) &&
        isMIDIfeatureEnabled(System::midiFeature_t::passToDIN))
    {
        // pass the message to din
        while (_midi.read(MIDI::interface_t::usb, MIDI::filterMode_t::fullDIN))
            processMessage(MIDI::interface_t::usb);
    }
    else
    {
        while (_midi.read(MIDI::interface_t::usb))
            processMessage(MIDI::interface_t::usb);
    }

    if (isMIDIfeatureEnabled(System::midiFeature_t::dinEnabled))
    {
        if (isMIDIfeatureEnabled(System::midiFeature_t::mergeEnabled))
        {
            auto mergeType = midiMergeType();

            switch (mergeType)
            {
            case System::midiMergeType_t::DINtoUSB:
            {
                // dump everything from DIN MIDI in to USB MIDI out
                while (_midi.read(MIDI::interface_t::din, MIDI::filterMode_t::fullUSB))
                    ;
            }
            break;

                // case System::midiMergeType_t::DINtoDIN:
                // loopback is automatically configured here
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
}

void System::run()
{
    checkComponents();
    checkMIDI();
    _hwa.update();
    _dmx.read();
    _scheduler.update();
}

void System::forceComponentRefresh()
{
    // extra check here - it's possible that preset was changed and then backup/restore procedure started
    // in that case this would get called
    if (_backupRestoreState == backupRestoreState_t::none)
    {
        _analog.update(true);
        _buttons.update(true);
    }
}