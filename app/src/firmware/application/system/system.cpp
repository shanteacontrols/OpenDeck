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

#include <zephyr/logging/log.h>
#include <zephyr/logging/log_ctrl.h>

using namespace io;
using namespace sys;
using namespace protocol;

namespace
{
    LOG_MODULE_REGISTER(system, CONFIG_OPENDECK_LOG_LEVEL);    // NOLINT
}    // namespace

#ifndef SW_VERSION_MAJOR
#define SW_VERSION_MAJOR 0
#endif

#ifndef SW_VERSION_MINOR
#define SW_VERSION_MINOR 0
#endif

#ifndef SW_VERSION_REVISION
#define SW_VERSION_REVISION 0
#endif

#ifndef PROJECT_TARGET_UID
#define PROJECT_TARGET_UID 0
#endif

System::System(Hwa&        hwa,
               Components& components)
    : _hwa(hwa)
    , _components(components)
    , _databaseHandlers(*this)
    , _sysExDataHandler(*this)
    , _presetChangeWork([this]()
                        {
                            messaging::SystemSignal signal = {};
                            signal.systemMessage           = messaging::systemMessage_t::PRESET_CHANGED;
                            signal.value                   = _components.database().currentPreset();

                            messaging::publish(signal);
                            forceComponentRefresh();
                        })
    , _forcedRefreshWork([this]()
                         {
                             forceComponentRefresh();
                         })
    , _sysExConf(_sysExDataHandler, SYS_EX_MID)
{
    messaging::subscribe<messaging::UmpSignal>(
        [this](const messaging::UmpSignal& event)
        {
            if (event.direction != messaging::MidiDirection::In)
            {
                return;
            }

            const auto message = midi::decode_message(event.packet);

            switch (message.type)
            {
            case midi::messageType_t::PROGRAM_CHANGE:
            {
                if (_components.database().read(database::Config::Section::common_t::COMMON_SETTINGS,
                                                Config::systemSetting_t::ENABLE_PRESET_CHANGE_WITH_PROGRAM_CHANGE_IN))
                {
                    _components.database().setPreset(message.data1);
                }
            }
            break;

            default:
                break;
            }
        });

    messaging::subscribe<messaging::UmpSignal>(
        [this](const messaging::UmpSignal& event)
        {
            if (event.direction != messaging::MidiDirection::In)
            {
                return;
            }

            _sysExConf.handle_packet(event.packet);

            if (_backupRestoreState == backupRestoreState_t::BACKUP)
            {
                backup();
            }
        });

    messaging::subscribe<messaging::SystemSignal>(
        [this](const messaging::SystemSignal& event)
        {
            switch (event.systemMessage)
            {
            case messaging::systemMessage_t::PRESET_CHANGE_INC_REQ:
            {
                _components.database().setPreset(_components.database().currentPreset() + 1);
            }
            break;

            case messaging::systemMessage_t::PRESET_CHANGE_DEC_REQ:
            {
                _components.database().setPreset(_components.database().currentPreset() - 1);
            }
            break;

            case messaging::systemMessage_t::PRESET_CHANGE_DIRECT_REQ:
            {
                _components.database().setPreset(event.value);
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
    LOG_INF("Starting init");

    _cInfo.registerHandler([this](size_t group, size_t index)
                           {
                               if (_sysExConf.is_configuration_enabled())
                               {
                                   std::array<uint16_t, 4> cInfoMessage = {
                                       SYSEX_CM_COMPONENT_ID,
                                       static_cast<uint16_t>(group),
                                       0,
                                       0
                                   };

                                   auto split = util::Conversion::Split14Bit(index);

                                   cInfoMessage[2] = split.high();
                                   cInfoMessage[3] = split.low();

                                   _sysExConf.send_custom_message(cInfoMessage);
                               }
                           });

    _hwa.registerOnUSBconnectionHandler([this]()
                                        {
                                            _forcedRefreshWork.reschedule(USB_CHANGE_FORCED_REFRESH_DELAY);
                                        });

    LOG_INF("Hwa init");

    if (!_hwa.init())
    {
        return false;
    }

    LOG_INF("Init database");

    if (!_components.database().init(_databaseHandlers))
    {
        return false;
    }

    LOG_INF("Init components");

    for (size_t i = 0; i < _components.io().size(); i++)
    {
        auto component = _components.io().at(i);

        if (component != nullptr)
        {
            component->init();
        }
    }

    _sysExConf.set_layout(_layout.layout());
    _sysExConf.setup_custom_requests(_layout.customRequests());

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
        messaging::MidiSignal signal = {};
        signal.source                = messaging::MidiSource::Program;
        signal.channel               = i;
        signal.index                 = MidiProgram.program(i);
        signal.value                 = 0;
        signal.message               = midi::messageType_t::PROGRAM_CHANGE;

        messaging::publish(signal);
    }

    messaging::SystemSignal initCompleteSignal = {};
    initCompleteSignal.systemMessage           = messaging::systemMessage_t::INIT_COMPLETE;
    messaging::publish(initCompleteSignal);

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
        static_cast<uint8_t>(lib::sysexconf::Wish::Backup),
        static_cast<uint8_t>(lib::sysexconf::Amount::All),
        0x00,    // block - set later in the loop
        0x00,    // section - set later in the loop
        0x00,    // index MSB - unused but required
        0x00,    // index LSB - unused but required
        0x00,    // new value MSB - unused but required
        0x00,    // new value LSB - unused but required
        0xF7
    };

    uint16_t presetChangeRequest[] = {
        static_cast<uint8_t>(lib::sysexconf::Wish::Set),
        static_cast<uint8_t>(lib::sysexconf::Amount::Single),
        static_cast<uint8_t>(sys::Config::block_t::GLOBAL),
        static_cast<uint8_t>(sys::Config::Section::global_t::SYSTEM_SETTINGS),
        0x00,    // index 0 (active preset) MSB
        0x00,    // index 0 (active preset) LSB
        0x00,    // preset value MSB - always 0
        0x00     // preset value LSB - set later in the loop
    };

    static constexpr uint8_t PRESET_CHANGE_REQUEST_PRESET_INDEX = 7;
    static constexpr uint8_t BACKUP_REQUEST_BLOCK_INDEX         = 8;
    static constexpr uint8_t BACKUP_REQUEST_SECTION_INDEX       = 9;

    uint8_t currentPreset = _components.database().currentPreset();

    // make sure not to report any errors while performing backup
    _sysExConf.set_user_error_ignore_mode(true);

    // first message sent as an response should be restore start marker
    // this is used to indicate that restore procedure is in progress
    uint16_t restoreMarker = SYSEX_CR_RESTORE_START;
    _sysExConf.send_custom_message(std::span<const uint16_t>(&restoreMarker, 1), false);

    // send internally created backup requests to sysex handler for all presets, blocks and presets
    for (uint8_t preset = 0; preset < _components.database().getSupportedPresets(); preset++)
    {
        _components.database().setPreset(preset);
        presetChangeRequest[PRESET_CHANGE_REQUEST_PRESET_INDEX] = preset;
        _sysExConf.send_custom_message(presetChangeRequest, false);

        for (size_t block = 0; block < _layout.blocks(); block++)
        {
            backupRequest[BACKUP_REQUEST_BLOCK_INDEX] = block;

            for (size_t section = 0; section < _layout.sections(block); section++)
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
                lib::midi::write_sysex7_payload_as_ump_packets(
                    lib::midi::DEFAULT_RX_GROUP,
                    std::span<const uint8_t>(&backupRequest[1], sizeof(backupRequest) - 2),
                    [this](const midi_ump& packet)
                    {
                        _sysExConf.handle_packet(packet);
                        return true;
                    });
            }
        }
    }

    _components.database().setPreset(currentPreset);
    presetChangeRequest[PRESET_CHANGE_REQUEST_PRESET_INDEX] = currentPreset;
    _sysExConf.send_custom_message(presetChangeRequest, false);

    // mark the end of restore procedure
    restoreMarker = SYSEX_CR_RESTORE_END;
    _sysExConf.send_custom_message(std::span<const uint16_t>(&restoreMarker, 1), false);

    // finally, send back full backup request to mark the end of sending
    uint16_t endMarker = SYSEX_CR_FULL_BACKUP;
    _sysExConf.send_custom_message(std::span<const uint16_t>(&endMarker, 1));
    _sysExConf.set_user_error_ignore_mode(false);

    _backupRestoreState = backupRestoreState_t::NONE;
}

ioComponent_t System::checkComponents()
{
    switch (_componentIndex)
    {
    case ioComponent_t::DIGITAL:
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
    {
        _componentIndex = ioComponent_t::INDICATORS;
    }
    break;

    case ioComponent_t::INDICATORS:
    default:
    {
        _componentIndex = ioComponent_t::DIGITAL;
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
    if (_components.database().read(database::Config::Section::common_t::COMMON_SETTINGS,
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

    messaging::SystemSignal signal = {};
    signal.systemMessage           = messaging::systemMessage_t::FORCE_IO_REFRESH;

    messaging::publish(signal);
}

bool System::SysExDataHandler::send_response(const midi_ump& packet)
{
    messaging::UmpSignal signal = {};
    signal.direction            = messaging::MidiDirection::Out;
    signal.packet               = packet;
    messaging::publish(signal);
    return true;
}

lib::sysexconf::Status System::SysExDataHandler::custom_request(uint16_t request, CustomResponse& custom_response)
{
    auto result = lib::sysexconf::Status::Ack;

    auto appendSW = [&custom_response]()
    {
        custom_response.append(SW_VERSION_MAJOR);
        custom_response.append(SW_VERSION_MINOR);
        custom_response.append(SW_VERSION_REVISION);
    };

    auto appendHW = [&custom_response]()
    {
        custom_response.append((PROJECT_TARGET_UID >> 24) & static_cast<uint32_t>(0xFF));
        custom_response.append((PROJECT_TARGET_UID >> 16) & static_cast<uint32_t>(0xFF));
        custom_response.append((PROJECT_TARGET_UID >> 8) & static_cast<uint32_t>(0xFF));
        custom_response.append(PROJECT_TARGET_UID & static_cast<uint32_t>(0xFF));
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
            result = lib::sysexconf::Status::ErrorWrite;
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
        custom_response.append(buttons::Collection::SIZE());
        custom_response.append(encoders::Collection::SIZE());
        custom_response.append(analog::Collection::SIZE());
        custom_response.append(leds::Collection::SIZE());
        custom_response.append(touchscreen::Collection::SIZE());
    }
    break;

    case SYSEX_CR_SUPPORTED_PRESETS:
    {
        custom_response.append(_system._components.database().getSupportedPresets());
    }
    break;

    case SYSEX_CR_BOOTLOADER_SUPPORT:
    {
        custom_response.append(1);
    }
    break;

    case SYSEX_CR_FULL_BACKUP:
    {
        // no response here, just set flag internally that backup needs to be done
        _system._backupRestoreState = backupRestoreState_t::BACKUP;

        messaging::SystemSignal signal = {};
        signal.systemMessage           = messaging::systemMessage_t::BACKUP;
        messaging::publish(signal);
    }
    break;

    case SYSEX_CR_RESTORE_START:
    {
        _system._backupRestoreState = backupRestoreState_t::RESTORE;
        _system._sysExConf.set_user_error_ignore_mode(true);

        messaging::SystemSignal signal = {};
        signal.systemMessage           = messaging::systemMessage_t::RESTORE_START;
        messaging::publish(signal);
    }
    break;

    case SYSEX_CR_RESTORE_END:
    {
        _system._backupRestoreState = backupRestoreState_t::NONE;
        _system._sysExConf.set_user_error_ignore_mode(false);

        messaging::SystemSignal signal = {};
        signal.systemMessage           = messaging::systemMessage_t::RESTORE_END;
        messaging::publish(signal);
    }
    break;

    default:
    {
        result = lib::sysexconf::Status::ErrorNotSupported;
    }
    break;
    }

    return result;
}

lib::sysexconf::Status System::SysExDataHandler::get(uint8_t   block,
                                                     uint8_t   section,
                                                     uint16_t  index,
                                                     uint16_t& value)
{
    return static_cast<lib::sysexconf::Status>(ConfigHandler.get(static_cast<sys::Config::block_t>(block), section, index, value));
}

lib::sysexconf::Status System::SysExDataHandler::set(uint8_t  block,
                                                     uint8_t  section,
                                                     uint16_t index,
                                                     uint16_t value)
{
    return static_cast<lib::sysexconf::Status>(ConfigHandler.set(static_cast<sys::Config::block_t>(block), section, index, value));
}

void System::DatabaseHandlers::presetChange(uint8_t preset)
{
    if (_system._backupRestoreState == backupRestoreState_t::NONE)
    {
        _system._presetChangeWork.reschedule(PRESET_CHANGE_NOTIFY_DELAY);
    }
}

void System::DatabaseHandlers::initialized()
{
    // nothing to do here
}

void System::DatabaseHandlers::factoryResetStart()
{
    LOG_INF("Starting factory reset");

    // Make sure all the buffered log lines are flushed before starting intensive flash writes.
    LOG_PANIC();

    messaging::SystemSignal signal = {};
    signal.systemMessage           = messaging::systemMessage_t::FACTORY_RESET_START;

    messaging::publish(signal);
}

void System::DatabaseHandlers::factoryResetDone()
{
    LOG_INF("Factory reset done, rebooting");

    messaging::SystemSignal signal = {};
    signal.systemMessage           = messaging::systemMessage_t::FACTORY_RESET_END;

    messaging::publish(signal);

    k_msleep(100);
    _system._hwa.reboot(fw_selector::fwType_t::APPLICATION);
}

std::optional<uint8_t> System::sysConfigGet(sys::Config::Section::global_t section, size_t index, uint16_t& value)
{
    if (section != sys::Config::Section::global_t::SYSTEM_SETTINGS)
    {
        return {};
    }

    switch (index)
    {
    case static_cast<size_t>(sys::Config::systemSetting_t::DISABLE_FORCED_REFRESH_AFTER_PRESET_CHANGE):
    case static_cast<size_t>(sys::Config::systemSetting_t::ENABLE_PRESET_CHANGE_WITH_PROGRAM_CHANGE_IN):
        break;

    default:
        return {};
    }

    uint32_t readValue = 0;

    auto result = _components.database().read(database::Config::Section::common_t::COMMON_SETTINGS, index, readValue)
                      ? sys::Config::Status::ACK
                      : sys::Config::Status::ERROR_READ;

    value = readValue;

    return result;
}

std::optional<uint8_t> System::sysConfigSet(sys::Config::Section::global_t section, size_t index, uint16_t value)
{
    if (section != sys::Config::Section::global_t::SYSTEM_SETTINGS)
    {
        return {};
    }

    switch (index)
    {
    case static_cast<size_t>(sys::Config::systemSetting_t::DISABLE_FORCED_REFRESH_AFTER_PRESET_CHANGE):
    case static_cast<size_t>(sys::Config::systemSetting_t::ENABLE_PRESET_CHANGE_WITH_PROGRAM_CHANGE_IN):
        break;

    default:
        return {};
    }

    auto result = _components.database().update(database::Config::Section::common_t::COMMON_SETTINGS, index, value)
                      ? sys::Config::Status::ACK
                      : sys::Config::Status::ERROR_WRITE;

    return result;
}
