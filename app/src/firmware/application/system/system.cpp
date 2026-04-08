/*

Copyright Igor Petrovic

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

#include "system.h"
#include "application/messaging/messaging.h"
#include "application/util/configurable/configurable.h"
#include "application/util/conversion/conversion.h"
#include "application/global/midi_program.h"
#include "bootloader/fw_selector/fw_selector.h"

#include "core/mcu.h"
#include "core/util/util.h"

using namespace io;
using namespace sys;
using namespace protocol;

System::System(Hwa&        hwa,
               Components& components)
    : _hwa(hwa)
    , _components(components)
    , _databaseHandlers(*this)
    , _sysExDataHandler(*this)
    , _sysExConf(
          _sysExDataHandler,
          SYS_EX_MID)
{
    MidiDispatcher.listen(messaging::eventType_t::MIDI_IN,
                          [this](const messaging::Event& event)
                          {
                              switch (event.message)
                              {
                              case midi::messageType_t::PROGRAM_CHANGE:
                              {
                                  if (_components.database().read(database::Config::Section::system_t::SYSTEM_SETTINGS,
                                                                  Config::systemSetting_t::ENABLE_PRESET_CHANGE_WITH_PROGRAM_CHANGE_IN))
                                  {
                                      _components.database().setPreset(event.index);
                                  }
                              }
                              break;

                              case midi::messageType_t::SYS_EX:
                              {
                                  _sysExConf.handleMessage(event.sysEx, event.sysExLength);

                                  if (_backupRestoreState == backupRestoreState_t::BACKUP)
                                  {
                                      backup();
                                  }
                              }
                              break;

                              default:
                                  break;
                              }
                          });

    MidiDispatcher.listen(messaging::eventType_t::SYSTEM,
                          [this](const messaging::Event& event)
                          {
                              switch (event.systemMessage)
                              {
                              case messaging::systemMessage_t::PRESET_CHANGE_INC_REQ:
                              {
                                  _components.database().setPreset(_components.database().getPreset() + 1);
                              }
                              break;

                              case messaging::systemMessage_t::PRESET_CHANGE_DEC_REQ:
                              {
                                  _components.database().setPreset(_components.database().getPreset() - 1);
                              }
                              break;

                              case messaging::systemMessage_t::PRESET_CHANGE_DIRECT_REQ:
                              {
                                  _components.database().setPreset(event.index);
                              }
                              break;

                              default:
                                  break;
                              }
                          });

    ConfigHandler.registerConfig(
        sys::Config::block_t::GLOBAL,
        // read
        [this](uint8_t section, size_t index, uint16_t& value)
        {
            return sysConfigGet(static_cast<sys::Config::Section::global_t>(section), index, value);
        },

        // write
        [this](uint8_t section, size_t index, uint16_t value)
        {
            return sysConfigSet(static_cast<sys::Config::Section::global_t>(section), index, value);
        });
}

bool System::init()
{
    _scheduler.init();

    _cInfo.registerHandler([this](size_t group, size_t index)
                           {
                               if (_sysExConf.isConfigurationEnabled())
                               {
                                   uint16_t cInfoMessage[] = {
                                       SYSEX_CM_COMPONENT_ID,
                                       static_cast<uint16_t>(group),
                                       0,
                                       0
                                   };

                                   auto split = util::Conversion::Split14Bit(index);

                                   cInfoMessage[2] = split.high();
                                   cInfoMessage[3] = split.low();

                                   _sysExConf.sendCustomMessage(cInfoMessage, 4);
                               }
                           });

    _hwa.registerOnUSBconnectionHandler([this]()
                                        {
                                            _scheduler.registerTask({ SCHEDULED_TASK_FORCED_REFRESH,
                                                                      USB_CHANGE_FORCED_REFRESH_DELAY,
                                                                      [this]()
                                                                      {
                                                                          forceComponentRefresh();
                                                                      } });
                                        });

    if (!_hwa.init())
    {
        return false;
    }

    if (!_components.database().init(_databaseHandlers))
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

    _sysExConf.setLayout(_layout.layout());
    _sysExConf.setupCustomRequests(_layout.customRequests());

    for (size_t i = 0; i < _components.protocol().size(); i++)
    {
        auto component = _components.protocol().at(i);

        if (component != nullptr)
        {
            component->init();
        }
    }

    // on startup, indicate current program for all channels
    for (int i = 1; i <= 16; i++)
    {
        messaging::Event event = {};
        event.componentIndex   = 0;
        event.channel          = i;
        event.index            = MidiProgram.program(i);
        event.value            = 0;
        event.message          = midi::messageType_t::PROGRAM_CHANGE;

        MidiDispatcher.notify(messaging::eventType_t::PROGRAM, event);
    }

    return true;
}

// Return the last processed IO component:
// components aren't checked all at once, but
// rather a single one for each run() call. This is
// done to reduce the amount of spent time inside checkComponents.
ioComponent_t System::run()
{
    _hwa.update();
    auto retVal = checkComponents();
    checkProtocols();
    _scheduler.update();

    return retVal;
}

void System::backup()
{
    uint8_t backupRequest[] = {
        0xF0,
        SYS_EX_MID.id1,
        SYS_EX_MID.id2,
        SYS_EX_MID.id3,
        0x00,    // request
        0x7F,    // all message parts,
        static_cast<uint8_t>(lib::sysexconf::wish_t::BACKUP),
        static_cast<uint8_t>(lib::sysexconf::amount_t::ALL),
        0x00,    // block - set later in the loop
        0x00,    // section - set later in the loop
        0x00,    // index MSB - unused but required
        0x00,    // index LSB - unused but required
        0x00,    // new value MSB - unused but required
        0x00,    // new value LSB - unused but required
        0xF7
    };

    uint16_t presetChangeRequest[] = {
        static_cast<uint8_t>(lib::sysexconf::wish_t::SET),
        static_cast<uint8_t>(lib::sysexconf::amount_t::SINGLE),
        static_cast<uint8_t>(sys::Config::block_t::GLOBAL),
        static_cast<uint8_t>(sys::Config::Section::global_t::SYSTEM_SETTINGS),
        0x00,    // index 0 (active preset) MSB
        0x00,    // index 0 (active preset) LSB
        0x00,    // preset value MSB - always 0
        0x00     // preset value LSB - set later in the loop
    };

    static constexpr uint8_t PRESET_CHANGE_REQUEST_SIZE         = 8;
    static constexpr uint8_t PRESET_CHANGE_REQUEST_PRESET_INDEX = 7;
    static constexpr uint8_t BACKUP_REQUEST_BLOCK_INDEX         = 8;
    static constexpr uint8_t BACKUP_REQUEST_SECTION_INDEX       = 9;

    uint8_t currentPreset = _components.database().getPreset();

    // make sure not to report any errors while performing backup
    _sysExConf.setUserErrorIgnoreMode(true);

    // first message sent as an response should be restore start marker
    // this is used to indicate that restore procedure is in progress
    uint16_t restoreMarker = SYSEX_CR_RESTORE_START;
    _sysExConf.sendCustomMessage(&restoreMarker, 1, false);

    // send internally created backup requests to sysex handler for all presets, blocks and presets
    for (uint8_t preset = 0; preset < _components.database().getSupportedPresets(); preset++)
    {
        _components.database().setPreset(preset);
        presetChangeRequest[PRESET_CHANGE_REQUEST_PRESET_INDEX] = preset;
        _sysExConf.sendCustomMessage(presetChangeRequest, PRESET_CHANGE_REQUEST_SIZE, false);

        for (size_t block = 0; block < _sysExConf.blocks(); block++)
        {
            backupRequest[BACKUP_REQUEST_BLOCK_INDEX] = block;

            for (size_t section = 0; section < _sysExConf.sections(block); section++)
            {
                // some sections are irrelevant for backup and should therefore be skipped

                if (
                    (block == static_cast<uint8_t>(sys::Config::block_t::LEDS)) &&
                    ((section == static_cast<uint8_t>(sys::Config::Section::leds_t::TEST_COLOR)) ||
                     (section == static_cast<uint8_t>(sys::Config::Section::leds_t::TEST_BLINK))))
                {
                    continue;
                }

                backupRequest[BACKUP_REQUEST_SECTION_INDEX] = section;
                _sysExConf.handleMessage(backupRequest, sizeof(backupRequest));
            }
        }
    }

    _components.database().setPreset(currentPreset);
    presetChangeRequest[PRESET_CHANGE_REQUEST_PRESET_INDEX] = currentPreset;
    _sysExConf.sendCustomMessage(presetChangeRequest, PRESET_CHANGE_REQUEST_SIZE, false);

    // mark the end of restore procedure
    restoreMarker = SYSEX_CR_RESTORE_END;
    _sysExConf.sendCustomMessage(&restoreMarker, 1, false);

    // finally, send back full backup request to mark the end of sending
    uint16_t endMarker = SYSEX_CR_FULL_BACKUP;
    _sysExConf.sendCustomMessage(&endMarker, 1);
    _sysExConf.setUserErrorIgnoreMode(false);

    _backupRestoreState = backupRestoreState_t::NONE;
}

ioComponent_t System::checkComponents()
{
    switch (_componentIndex)
    {
    case ioComponent_t::BUTTONS:
    {
        _componentIndex = ioComponent_t::ENCODERS;
    }
    break;

    case ioComponent_t::ENCODERS:
    {
        _componentIndex = ioComponent_t::ANALOG;
    }
    break;

    case ioComponent_t::ANALOG:
    {
        _componentIndex = ioComponent_t::LEDS;
    }
    break;

    case ioComponent_t::LEDS:
    {
        _componentIndex = ioComponent_t::I2C;
    }
    break;

    case ioComponent_t::I2C:
    {
        _componentIndex = ioComponent_t::TOUCHSCREEN;
    }
    break;

    case ioComponent_t::TOUCHSCREEN:
    default:
    {
        _componentIndex = ioComponent_t::BUTTONS;
    }
    break;
    }

    // For each component, allow up to MAX_UPDATES_PER_RUN updates:
    // This is done so that no single component update takes too long, and
    // thus making other things wait.

    auto component         = _components.io().at(static_cast<size_t>(_componentIndex));
    auto maxComponentIndex = component->maxComponentUpdateIndex();
    auto loopIterations    = maxComponentIndex >= MAX_UPDATES_PER_RUN ? MAX_UPDATES_PER_RUN : !maxComponentIndex ? 1
                                                                                                                 : maxComponentIndex;

    if (component != nullptr)
    {
        for (size_t i = 0; i < loopIterations; i++)
        {
            component->updateSingle(_componentUpdateIndex[static_cast<size_t>(_componentIndex)]);

            if (++_componentUpdateIndex[static_cast<size_t>(_componentIndex)] >= maxComponentIndex)
            {
                _componentUpdateIndex[static_cast<size_t>(_componentIndex)] = 0;
            }
        }
    }

    // return the last processed io component
    return _componentIndex;
}

void System::checkProtocols()
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

void System::forceComponentRefresh()
{
    if (_components.database().read(database::Config::Section::system_t::SYSTEM_SETTINGS,
                                    Config::systemSetting_t::DISABLE_FORCED_REFRESH_AFTER_PRESET_CHANGE))
    {
        return;
    }

    // extra check here - it's possible that preset was changed and then backup/restore procedure started
    // in that case this would get called
    if (_backupRestoreState != backupRestoreState_t::NONE)
    {
        return;
    }

    messaging::Event event = {};
    event.systemMessage    = messaging::systemMessage_t::FORCE_IO_REFRESH;

    MidiDispatcher.notify(messaging::eventType_t::SYSTEM, event);
}

void System::SysExDataHandler::sendResponse(uint8_t* array, uint16_t size)
{
    messaging::Event event = {};
    event.systemMessage    = messaging::systemMessage_t::SYS_EX_RESPONSE;
    event.message          = midi::messageType_t::SYS_EX;
    event.sysEx            = array;
    event.sysExLength      = size;

    MidiDispatcher.notify(messaging::eventType_t::SYSTEM, event);
}

uint8_t System::SysExDataHandler::customRequest(uint16_t request, CustomResponse& customResponse)
{
    uint8_t result = sys::Config::Status::ACK;

    auto appendSW = [&customResponse]()
    {
        customResponse.append(SW_VERSION_MAJOR);
        customResponse.append(SW_VERSION_MINOR);
        customResponse.append(SW_VERSION_REVISION);
    };

    auto appendHW = [&customResponse]()
    {
        customResponse.append((PROJECT_TARGET_UID >> 24) & static_cast<uint32_t>(0xFF));
        customResponse.append((PROJECT_TARGET_UID >> 16) & static_cast<uint32_t>(0xFF));
        customResponse.append((PROJECT_TARGET_UID >> 8) & static_cast<uint32_t>(0xFF));
        customResponse.append(PROJECT_TARGET_UID & static_cast<uint32_t>(0xFF));
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
            result = static_cast<uint8_t>(lib::sysexconf::status_t::ERROR_WRITE);
        }
    }
    break;

    case SYSEX_CR_REBOOT_APP:
    {
        _system._hwa.reboot(fw_selector::fwType_t::APPLICATION);
    }
    break;

    case SYSEX_CR_REBOOT_BTLDR:
    {
        _system._hwa.reboot(fw_selector::fwType_t::BOOTLOADER);
    }
    break;

    case SYSEX_CR_MAX_COMPONENTS:
    {
        customResponse.append(buttons::Collection::SIZE());
        customResponse.append(encoders::Collection::SIZE());
        customResponse.append(analog::Collection::SIZE());
        customResponse.append(leds::Collection::SIZE());
        customResponse.append(touchscreen::Collection::SIZE());
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
        _system._backupRestoreState = backupRestoreState_t::BACKUP;

        messaging::Event event = {};
        event.componentIndex   = 0;
        event.channel          = 0;
        event.index            = 0;
        event.value            = 0;
        event.systemMessage    = messaging::systemMessage_t::BACKUP;

        MidiDispatcher.notify(messaging::eventType_t::SYSTEM, event);
    }
    break;

    case SYSEX_CR_RESTORE_START:
    {
        _system._backupRestoreState = backupRestoreState_t::RESTORE;
        _system._sysExConf.setUserErrorIgnoreMode(true);

        messaging::Event event = {};
        event.componentIndex   = 0;
        event.channel          = 0;
        event.index            = 0;
        event.value            = 0;
        event.systemMessage    = messaging::systemMessage_t::RESTORE_START;

        MidiDispatcher.notify(messaging::eventType_t::SYSTEM, event);
    }
    break;

    case SYSEX_CR_RESTORE_END:
    {
        _system._backupRestoreState = backupRestoreState_t::NONE;
        _system._sysExConf.setUserErrorIgnoreMode(false);

        messaging::Event event = {};
        event.componentIndex   = 0;
        event.channel          = 0;
        event.index            = 0;
        event.value            = 0;
        event.systemMessage    = messaging::systemMessage_t::RESTORE_END;

        MidiDispatcher.notify(messaging::eventType_t::SYSTEM, event);
    }
    break;

    default:
    {
        result = sys::Config::Status::ERROR_NOT_SUPPORTED;
    }
    break;
    }

    return result;
}

uint8_t System::SysExDataHandler::get(uint8_t   block,
                                      uint8_t   section,
                                      uint16_t  index,
                                      uint16_t& value)
{
    return ConfigHandler.get(static_cast<sys::Config::block_t>(block), section, index, value);
}

uint8_t System::SysExDataHandler::set(uint8_t  block,
                                      uint8_t  section,
                                      uint16_t index,
                                      uint16_t value)
{
    return ConfigHandler.set(static_cast<sys::Config::block_t>(block), section, index, value);
}

void System::DatabaseHandlers::presetChange(uint8_t preset)
{
    if (_system._backupRestoreState == backupRestoreState_t::NONE)
    {
        _system._scheduler.registerTask({ SCHEDULED_TASK_PRESET,
                                          PRESET_CHANGE_NOTIFY_DELAY,
                                          [&]()
                                          {
                                              messaging::Event event = {};
                                              event.componentIndex   = 0;
                                              event.channel          = 0;
                                              event.index            = _system._components.database().getPreset();
                                              event.value            = 0;
                                              event.systemMessage    = messaging::systemMessage_t::PRESET_CHANGED;

                                              MidiDispatcher.notify(messaging::eventType_t::SYSTEM, event);

                                              _system.forceComponentRefresh();
                                          } });
    }
}

void System::DatabaseHandlers::initialized()
{
    // nothing to do here
}

void System::DatabaseHandlers::factoryResetStart()
{
    messaging::Event event = {};
    event.componentIndex   = 0;
    event.channel          = 0;
    event.index            = 0;
    event.value            = 0;
    event.systemMessage    = messaging::systemMessage_t::FACTORY_RESET_START;

    MidiDispatcher.notify(messaging::eventType_t::SYSTEM, event);
}

void System::DatabaseHandlers::factoryResetDone()
{
    messaging::Event event = {};
    event.componentIndex   = 0;
    event.channel          = 0;
    event.index            = 0;
    event.value            = 0;
    event.systemMessage    = messaging::systemMessage_t::FACTORY_RESET_END;

    MidiDispatcher.notify(messaging::eventType_t::SYSTEM, event);
}

std::optional<uint8_t> System::sysConfigGet(sys::Config::Section::global_t section, size_t index, uint16_t& value)
{
    if (section != sys::Config::Section::global_t::SYSTEM_SETTINGS)
    {
        return std::nullopt;
    }

    switch (index)
    {
    case static_cast<size_t>(sys::Config::systemSetting_t::DISABLE_FORCED_REFRESH_AFTER_PRESET_CHANGE):
    case static_cast<size_t>(sys::Config::systemSetting_t::ENABLE_PRESET_CHANGE_WITH_PROGRAM_CHANGE_IN):
        break;

    default:
        return std::nullopt;
    }

    uint32_t readValue;

    auto result = _components.database().read(database::Config::Section::system_t::SYSTEM_SETTINGS, index, readValue)
                      ? sys::Config::Status::ACK
                      : sys::Config::Status::ERROR_READ;

    value = readValue;

    return result;
}

std::optional<uint8_t> System::sysConfigSet(sys::Config::Section::global_t section, size_t index, uint16_t value)
{
    if (section != sys::Config::Section::global_t::SYSTEM_SETTINGS)
    {
        return std::nullopt;
    }

    switch (index)
    {
    case static_cast<size_t>(sys::Config::systemSetting_t::DISABLE_FORCED_REFRESH_AFTER_PRESET_CHANGE):
    case static_cast<size_t>(sys::Config::systemSetting_t::ENABLE_PRESET_CHANGE_WITH_PROGRAM_CHANGE_IN):
        break;

    default:
        return std::nullopt;
    }

    auto result = _components.database().update(database::Config::Section::system_t::SYSTEM_SETTINGS, index, value)
                      ? sys::Config::Status::ACK
                      : sys::Config::Status::ERROR_WRITE;

    return result;
}