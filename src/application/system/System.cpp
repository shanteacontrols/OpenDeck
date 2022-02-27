/*

Copyright 2015-2022 Igor Petrovic

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
#include "Layout.h"
#include "bootloader/FwSelector/FwSelector.h"
#include "core/src/general/Timing.h"
#include "core/src/general/Helpers.h"
#include "system/Config.h"
#include "messaging/Messaging.h"
#include "util/configurable/Configurable.h"
#include "util/conversion/Conversion.h"
#include "io/common/Common.h"

using namespace IO;
using namespace System;

Instance::Instance(HWA&        hwa,
                   Components& components)
    : _hwa(hwa)
    , _components(components)
    , _dbHandlers(*this)
    , _sysExDataHandler(*this)
    , _sysExConf(
          _sysExDataHandler,
          _sysExMID)
{
    MIDIDispatcher.listen(Messaging::eventSource_t::midiIn,
                          Messaging::listenType_t::all,
                          [this](const Messaging::event_t& event) {
                              if (event.message == MIDI::messageType_t::systemExclusive)
                              {
                                  _sysExConf.handleMessage(event.sysEx, event.sysExLength);

                                  if (_backupRestoreState == backupRestoreState_t::backup)
                                  {
                                      backup();
                                  }
                              }
                          });
}

bool Instance::init()
{
    _scheduler.init();

    _cInfo.registerHandler([this](size_t group, size_t index) {
        if (_sysExConf.isConfigurationEnabled())
        {
            uint16_t cInfoMessage[] = {
                SYSEX_CM_COMPONENT_ID,
                static_cast<uint16_t>(group),
                0,
                0
            };

            auto split = Util::Conversion::Split14bit(index);

            cInfoMessage[2] = split.high();
            cInfoMessage[3] = split.low();

            _sysExConf.sendCustomMessage(cInfoMessage, 4);
        }
    });

    _hwa.registerOnUSBconnectionHandler([this]() {
        _scheduler.registerTask({ SCHEDULED_TASK_FORCED_REFRESH,
                                  USB_CHANGE_FORCED_REFRESH_DELAY,
                                  [this]() { forceComponentRefresh(); } });
    });

    if (!_hwa.init())
    {
        return false;
    }

    if (!_components.database().init(_dbHandlers))
    {
        return false;
    }

    for (size_t i = 0; i < _components.io().size(); i++)
    {
        auto component = _components.io().at(i);

        if (component != nullptr)
        {
            component->init();
        }
    }

    _sysExConf.setLayout(sysExLayout);
    _sysExConf.setupCustomRequests(customRequests);

    for (size_t i = 0; i < _components.protocol().size(); i++)
    {
        auto component = _components.protocol().at(i);

        if (component != nullptr)
        {
            component->init();
        }
    }

    // on startup, indicate current program for all channels (if any leds have program change assigned as control mode)
    for (int i = 0; i < 16; i++)
    {
        Messaging::event_t event;

        event.componentIndex = static_cast<uint8_t>(Messaging::systemMessage_t::midiProgramIndication);
        event.midiChannel    = i;
        event.midiIndex      = Common::program(i);
        event.midiValue      = 0;
        event.message        = ::MIDI::messageType_t::programChange;

        MIDIDispatcher.notify(Messaging::eventSource_t::system,
                              event,
                              Messaging::listenType_t::nonFwd);
    }

    return true;
}

void Instance::run()
{
    _hwa.update();
    checkComponents();
    checkProtocols();
    _scheduler.update();
}

void Instance::backup()
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
        static_cast<uint8_t>(System::Config::block_t::global),
        static_cast<uint8_t>(System::Config::Section::global_t::presets),
        0x00,    // index 0 (active preset) MSB
        0x00,    // index 0 (active preset) LSB
        0x00,    // preset value MSB - always 0
        0x00     // preset value LSB - set later in the loop
    };

    const uint8_t presetChangeRequestSize        = 8;
    const uint8_t presetChangeRequestPresetIndex = 7;
    const uint8_t backupRequestBlockIndex        = 8;
    const uint8_t backupRequestSectionIndex      = 9;

    uint8_t currentPreset = _components.database().getPreset();

    // make sure not to report any errors while performing backup
    _sysExConf.setSilentMode(true);

    // first message sent as an response should be restore start marker
    // this is used to indicate that restore procedure is in progress
    uint16_t restoreMarker = SYSEX_CR_RESTORE_START;
    _sysExConf.sendCustomMessage(&restoreMarker, 1, false);

    // send internally created backup requests to sysex handler for all presets, blocks and presets
    for (uint8_t preset = 0; preset < _components.database().getSupportedPresets(); preset++)
    {
        _components.database().setPreset(preset);
        presetChangeRequest[presetChangeRequestPresetIndex] = preset;
        _sysExConf.sendCustomMessage(presetChangeRequest, presetChangeRequestSize, false);

        for (size_t block = 0; block < _sysExConf.blocks(); block++)
        {
            backupRequest[backupRequestBlockIndex] = block;

            for (size_t section = 0; section < _sysExConf.sections(block); section++)
            {
                if (
                    (block == static_cast<uint8_t>(System::Config::block_t::leds)) &&
                    ((section == static_cast<uint8_t>(System::Config::Section::leds_t::testColor)) ||
                     (section == static_cast<uint8_t>(System::Config::Section::leds_t::testBlink))))
                {
                    continue;    // testing sections, skip
                }

                backupRequest[backupRequestSectionIndex] = section;
                _sysExConf.handleMessage(backupRequest, sizeof(backupRequest));
            }
        }
    }

    _components.database().setPreset(currentPreset);
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

void Instance::checkComponents()
{
    static ioComponent_t    componentIndex                                                    = ioComponent_t::buttons;
    static size_t           componentUpdateIndex[static_cast<uint8_t>(ioComponent_t::AMOUNT)] = {};
    static constexpr size_t MAX_UPDATES_PER_CHECK                                             = 16;

    switch (componentIndex)
    {
    case ioComponent_t::buttons:
    {
        componentIndex = ioComponent_t::encoders;
    }
    break;

    case ioComponent_t::encoders:
    {
        componentIndex = ioComponent_t::analog;
    }
    break;

    case ioComponent_t::analog:
    {
        componentIndex = ioComponent_t::leds;
    }
    break;

    case ioComponent_t::leds:
    {
        componentIndex = ioComponent_t::i2c;
    }
    break;

    case ioComponent_t::i2c:
    {
        componentIndex = ioComponent_t::touchscreen;
    }
    break;

    case ioComponent_t::touchscreen:
    default:
    {
        componentIndex = ioComponent_t::buttons;
    }
    break;
    }

    // For each component, allow up to MAX_UPDATES_PER_CHECK updates:
    // This is done so that no single component update takes too long, and
    // thus making other things wait.

    auto component         = _components.io().at(static_cast<size_t>(componentIndex));
    auto maxComponentIndex = component->maxComponentUpdateIndex();
    auto loopIterations    = maxComponentIndex >= MAX_UPDATES_PER_CHECK ? MAX_UPDATES_PER_CHECK : !maxComponentIndex ? 1
                                                                                                                     : maxComponentIndex;

    if (component != nullptr)
    {
        for (size_t i = 0; i < loopIterations; i++)
        {
            component->updateSingle(componentUpdateIndex[static_cast<size_t>(componentIndex)]);

            if (++componentUpdateIndex[static_cast<size_t>(componentIndex)] >= maxComponentIndex)
            {
                componentUpdateIndex[static_cast<size_t>(componentIndex)] = 0;
            }
        }
    }
}

void Instance::checkProtocols()
{
    for (size_t i = 0; i < _components.protocol().size(); i++)
    {
        auto component = _components.protocol().at(i);

        if (component != nullptr)
        {
            component->read();
        }
    }
}

void Instance::forceComponentRefresh()
{
    // extra check here - it's possible that preset was changed and then backup/restore procedure started
    // in that case this would get called
    if (_backupRestoreState == backupRestoreState_t::none)
    {
        Messaging::event_t event;
        event.componentIndex = static_cast<uint8_t>(Messaging::systemMessage_t::forceIOrefresh);

        MIDIDispatcher.notify(Messaging::eventSource_t::system,
                              event,
                              Messaging::listenType_t::all);
    }
}

void Instance::SysExDataHandler::sendResponse(uint8_t* array, uint16_t size)
{
    Messaging::event_t event;
    event.componentIndex = static_cast<uint16_t>(Messaging::systemMessage_t::sysExResponse);
    event.message        = MIDI::messageType_t::systemExclusive;
    event.sysEx          = array;
    event.sysExLength    = size;

    MIDIDispatcher.notify(Messaging::eventSource_t::system,
                          event,
                          Messaging::listenType_t::nonFwd);
}

uint8_t Instance::SysExDataHandler::customRequest(uint16_t request, CustomResponse& customResponse)
{
    uint8_t result = System::Config::status_t::ack;

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
        if (!_system._components.database().factoryReset())
        {
            result = static_cast<uint8_t>(SysExConf::status_t::errorWrite);
        }
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
        customResponse.append(Buttons::Collection::size());
        customResponse.append(Encoders::Collection::size());
        customResponse.append(Analog::Collection::size());
        customResponse.append(LEDs::Collection::size());
        customResponse.append(Touchscreen::Collection::size());
    }
    break;

    case SYSEX_CR_SUPPORTED_PRESETS:
    {
        customResponse.append(_system._components.database().getSupportedPresets());
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
        result = System::Config::status_t::errorNotSupported;
    }
    break;
    }

    return result;
}

uint8_t Instance::SysExDataHandler::get(uint8_t   block,
                                        uint8_t   section,
                                        uint16_t  index,
                                        uint16_t& value)
{
    return ConfigHandler.get(static_cast<System::Config::block_t>(block), section, index, value);
}

uint8_t Instance::SysExDataHandler::set(uint8_t  block,
                                        uint8_t  section,
                                        uint16_t index,
                                        uint16_t value)
{
    return ConfigHandler.set(static_cast<System::Config::block_t>(block), section, index, value);
}

void Instance::DBhandlers::presetChange(uint8_t preset)
{
    if (_system._backupRestoreState == backupRestoreState_t::none)
    {
        _system._scheduler.registerTask({ SCHEDULED_TASK_PRESET,
                                          PRESET_CHANGE_NOTIFY_DELAY,
                                          [&]() {
        Messaging::event_t event;
        event.componentIndex = 0;
        event.midiChannel    = 0;
        event.midiIndex      = preset;
        event.midiValue      = 0;
        event.message        = MIDI::messageType_t::programChange;

        MIDIDispatcher.notify(Messaging::eventSource_t::preset,
                          event,
                          Messaging::listenType_t::nonFwd);

        _system.forceComponentRefresh(); } });
    }
}

void Instance::DBhandlers::initialized()
{
    // nothing to do here
}

void Instance::DBhandlers::factoryResetStart()
{
    // nothing to do here
}

void Instance::DBhandlers::factoryResetDone()
{
    // Don't run this if database isn't fully initialized yet
    // to avoid MCU reset if factory reset is needed on first run.
    if (_system._components.database().isInitialized())
    {
        _system._hwa.reboot(FwSelector::fwType_t::application);
    }
}
