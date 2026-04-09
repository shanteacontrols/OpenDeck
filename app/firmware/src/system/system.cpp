/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "system.h"
#include "messaging/messaging.h"
#include "util/configurable/configurable.h"
#include "util/conversion/conversion.h"
#include "global/midi_program.h"
#include "fw_selector/common.h"

#include <zephyr/logging/log.h>
#include <zephyr/logging/log_ctrl.h>

using namespace io;
using namespace sys;
using namespace protocol;

namespace
{
    LOG_MODULE_REGISTER(system, CONFIG_OPENDECK_LOG_LEVEL);    // NOLINT
}    // namespace

System::System(Hwa& hwa)
    : _hwa(hwa)
    , _database_handlers(*this)
    , _sysex_data_handler(*this)
    , _forced_refresh_work([this]()
                           {
                               force_component_refresh();
                           })
    , _io_resume_work([]
                      {
                          LOG_INF("Resuming IO threads");

                          io::Base::resume();

                          LOG_INF("Indicating current program for all channels");

                          static constexpr int MIDI_CHANNEL_COUNT = 16;

                          for (int i = 1; i <= MIDI_CHANNEL_COUNT; i++)
                          {
                              messaging::InternalProgram signal = {};
                              signal.channel                    = i;
                              signal.index                      = MidiProgram.program(i);
                              signal.value                      = 0;

                              messaging::publish(signal);
                          }
                      },
                      []()
                      {
                          return threads::SystemWorkqueue::handle();
                      })
    , _factory_reset_work([this]()
                          {
                              run_factory_reset();
                          },
                          []()
                          {
                              return threads::SystemWorkqueue::handle();
                          })
    , _backup_work([this]()
                   {
                       run_backup_step();
                   },
                   []()
                   {
                       return threads::SystemWorkqueue::handle();
                   })
    , _restore_work([this]()
                    {
                        run_restore_step();
                    },
                    []()
                    {
                        return threads::SystemWorkqueue::handle();
                    })
    , _reboot_work([this]()
                   {
                       _hwa.reboot(_reboot_type);
                   },
                   []()
                   {
                       return threads::SystemWorkqueue::handle();
                   })
    , _sysex_conf_close_work([this]()
                             {
                                 close_inactive_sysex_configuration_session();
                             },
                             []()
                             {
                                 return threads::SystemWorkqueue::handle();
                             })
    , _sysex_conf(_sysex_data_handler, SYS_EX_MANUFACTURER_ID)
{
    messaging::subscribe<messaging::UmpSignal>(
        [this](const messaging::UmpSignal& event)
        {
            if (event.direction != messaging::MidiDirection::In)
            {
                return;
            }

            if (zlibs::utils::midi::is_sysex7_packet(event.packet))
            {
                // Keep the previous session state so ConnOpen/ConnClose transitions can be detected.
                const bool was_configuration_enabled = _sysex_conf.is_configuration_enabled();

                if (_backup_restore_state == BackupRestoreState::Restore)
                {
                    [[maybe_unused]] auto ret = queue_restore_packet(event.packet);
                    return;
                }

                _sysex_conf.handle_packet(event.packet);
                update_sysex_configuration_session(was_configuration_enabled);
                return;
            }

            const auto message = midi::decode_message(event.packet);

            if ((message.type == midi::MessageType::ProgramChange) &&
                _hwa.database().read(database::Config::Section::Common::CommonSettings,
                                     Config::SystemSetting::EnablePresetChangeWithProgramChangeIn))
            {
                _hwa.database().set_preset(message.data1);
            }
        });

    messaging::subscribe<messaging::SystemSignal>(
        [this](const messaging::SystemSignal& event)
        {
            switch (event.system_message)
            {
            case messaging::SystemMessage::PresetChangeIncReq:
            {
                _hwa.database().set_preset(_hwa.database().current_preset() + 1);
            }
            break;

            case messaging::SystemMessage::PresetChangeDecReq:
            {
                _hwa.database().set_preset(_hwa.database().current_preset() - 1);
            }
            break;

            case messaging::SystemMessage::PresetChangeDirectReq:
            {
                _hwa.database().set_preset(event.value);
            }
            break;

            case messaging::SystemMessage::UsbMidiReady:
            {
                LOG_INF("USB ready, scheduling forced component refresh");
                schedule_forced_refresh(ForcedRefreshType::UsbInit, USB_CHANGE_FORCED_REFRESH_DELAY);
            }
            break;

            default:
                break;
            }
        });

    ConfigHandler.register_config(
        sys::Config::Block::Global,
        // read
        [this](uint8_t section, size_t index, uint16_t& value)
        {
            return sys_config_get(static_cast<sys::Config::Section::Global>(section), index, value);
        },

        // write
        [this](uint8_t section, size_t index, uint16_t value)
        {
            return sys_config_set(static_cast<sys::Config::Section::Global>(section), index, value);
        });
}

bool System::init()
{
    LOG_INF("Starting initialization");

    io::Base::freeze();

    LOG_INF("MCU: %s", CONFIG_SOC);
    LOG_INF("Zephyr board: %s", CONFIG_BOARD);
    LOG_INF("OpenDeck target: %s", OPENDECK_TARGET);
    LOG_INF("EMUEEPROM page size: %u", static_cast<unsigned>(CONFIG_ZLIBS_UTILS_EMUEEPROM_PAGE_SIZE));
    LOG_INF("Buttons: %u", static_cast<unsigned>(io::buttons::Collection::size(io::buttons::GroupDigitalInputs)));
    LOG_INF("Encoders: %u", static_cast<unsigned>(io::encoders::Collection::size()));
    LOG_INF("Analog: %u", static_cast<unsigned>(io::analog::Collection::size()));
    LOG_INF("LEDs: %u", static_cast<unsigned>(io::leds::Collection::size()));
    LOG_INF("Touchscreen components: %u", static_cast<unsigned>(io::touchscreen::Collection::size()));
    LOG_INF("DIN MIDI supported: %s", IS_ENABLED(CONFIG_PROJECT_TARGET_SUPPORT_DIN_MIDI) ? "Yes" : "No");
    LOG_INF("BLE MIDI supported: %s", IS_ENABLED(CONFIG_PROJECT_TARGET_SUPPORT_BLE) ? "Yes" : "No");

    _cinfo.register_handler([this](size_t group, size_t index)
                            {
                                if (_sysex_conf.is_configuration_enabled())
                                {
                                    std::array<uint16_t, 4> cinfo_message = {
                                        SYSEX_CM_COMPONENT_ID,
                                        static_cast<uint16_t>(group),
                                        0,
                                        0
                                    };

                                    auto split = util::Conversion::Split14Bit(index);

                                    cinfo_message[2] = split.high();
                                    cinfo_message[3] = split.low();

                                    _sysex_conf.send_custom_message(cinfo_message);
                                }
                            });

    LOG_INF("Hwa init");

    if (!_hwa.init())
    {
        return false;
    }

    LOG_INF("Init database");

    if (!_hwa.database().init(_database_handlers))
    {
        return false;
    }

    if (_reboot_work.is_pending())
    {
        LOG_INF("Reboot scheduled during database initialization, skipping component initialization");
        return true;
    }

    collection_init();

    LOG_INF("Init SysEx layout");

    _sysex_conf.set_layout(_layout.layout());
    _sysex_conf.setup_custom_requests(_layout.custom_requests());

    LOG_INF("Initialization complete");

    messaging::SystemSignal init_complete_signal = {};
    init_complete_signal.system_message          = messaging::SystemMessage::InitComplete;
    messaging::publish(init_complete_signal);

    // allow io threads to continue after their readings have been settled
    _io_resume_work.reschedule(INITIAL_IO_RESUME_DELAY_MS);

    return true;
}

void System::collection_init()
{
    LOG_INF("Init IO collection");

    for (size_t i = 0; i < _hwa.io().size(); i++)
    {
        auto component = _hwa.io().at(i);

        if (component != nullptr)
        {
            component->init();
        }
    }

    LOG_INF("Init protocol collection");

    for (size_t i = 0; i < _hwa.protocol().size(); i++)
    {
        auto component = _hwa.protocol().at(i);

        if (component != nullptr)
        {
            component->init();
        }
    }

    _components_initialized = true;
}

void System::collection_deinit()
{
    LOG_PANIC();
    LOG_INF("Deinit IO collection");

    for (auto* component : _hwa.io())
    {
        if (component != nullptr)
        {
            component->deinit();
        }
    }

    LOG_INF("Deinit protocol collection");

    for (auto* component : _hwa.protocol())
    {
        if (component != nullptr)
        {
            component->deinit();
        }
    }

    _components_initialized = false;
}

void System::start_backup()
{
    LOG_INF("Starting backup");

    _sysex_conf_close_work.cancel();

    messaging::SystemSignal signal = {};
    signal.system_message          = messaging::SystemMessage::BackupStart;
    messaging::publish(signal);

    _backup_restore_state = BackupRestoreState::Backup;

    _backup_session.active         = true;
    _backup_session.phase          = BackupPhase::RestoreStart;
    _backup_session.current_preset = _hwa.database().current_preset();
    _backup_session.preset_index   = 0;
    _backup_session.block_index    = 0;
    _backup_session.section_index  = 0;
    _backup_generated_packets      = 0;

    io::Base::freeze();
    _sysex_conf.set_user_error_ignore_mode(true);
    _backup_work.reschedule(BACKUP_PROCESSING_START_DELAY_MS);
}

void System::run_backup_step()
{
    if (!_backup_session.active)
    {
        return;
    }

    switch (_backup_session.phase)
    {
    case BackupPhase::RestoreStart:
    {
        uint16_t restore_marker = SYSEX_CR_RESTORE_START;
        _sysex_conf.send_custom_message(std::span<const uint16_t>(&restore_marker, 1), false);
        _backup_session.phase = BackupPhase::PresetSelect;
    }
    break;

    case BackupPhase::PresetSelect:
    {
        if (_backup_session.preset_index >= _hwa.database().supported_presets())
        {
            _backup_session.phase = BackupPhase::RestoreCurrentPreset;
            break;
        }

        LOG_INF("Backup preset %d", _backup_session.preset_index);
        _hwa.database().set_preset(_backup_session.preset_index);
        emit_preset_change(_backup_session.preset_index);
        _backup_session.block_index   = 0;
        _backup_session.section_index = 0;
        _backup_session.phase         = BackupPhase::PresetData;
    }
    break;

    case BackupPhase::PresetData:
    {
        auto block   = _backup_session.block_index;
        auto section = _backup_session.section_index;

        if (!find_next_backup_section(block, section))
        {
            _backup_session.preset_index++;
            _backup_session.phase = BackupPhase::PresetSelect;
            break;
        }

        [[maybe_unused]] auto ret = emit_backup_request(block, section);
        section++;
        _backup_session.block_index   = block;
        _backup_session.section_index = section;
    }
    break;

    case BackupPhase::RestoreCurrentPreset:
    {
        _hwa.database().set_preset(_backup_session.current_preset);
        emit_preset_change(_backup_session.current_preset);
        _backup_session.phase = BackupPhase::RestoreEnd;
    }
    break;

    case BackupPhase::RestoreEnd:
    {
        uint16_t restore_marker = SYSEX_CR_RESTORE_END;
        _sysex_conf.send_custom_message(std::span<const uint16_t>(&restore_marker, 1), false);
        _backup_session.phase = BackupPhase::FinalAck;
    }
    break;

    case BackupPhase::FinalAck:
    {
        uint16_t end_marker = SYSEX_CR_FULL_BACKUP;
        _sysex_conf.send_custom_message(std::span<const uint16_t>(&end_marker, 1));
        finish_backup();
        return;
    }

    case BackupPhase::Idle:
    default:
        return;
    }

    _backup_work.reschedule(BACKUP_RESTORE_STEP_DELAY_MS);
}

bool System::emit_backup_request(uint8_t block, uint8_t section)
{
    static constexpr uint8_t SYSEX_REQUEST   = 0x00;
    static constexpr uint8_t SYSEX_ALL_PARTS = 0x7F;

    uint8_t backup_request[] = {
        zlibs::utils::midi::SYS_EX_START,
        SYS_EX_MANUFACTURER_ID.id1,
        SYS_EX_MANUFACTURER_ID.id2,
        SYS_EX_MANUFACTURER_ID.id3,
        SYSEX_REQUEST,
        SYSEX_ALL_PARTS,
        static_cast<uint8_t>(zlibs::utils::sysex_conf::Wish::Backup),
        static_cast<uint8_t>(zlibs::utils::sysex_conf::Amount::All),
        block,
        section,
        0x00,    // index MSB - unused but required
        0x00,    // index LSB - unused but required
        0x00,    // new value MSB - unused but required
        0x00,    // new value LSB - unused but required
        zlibs::utils::midi::SYS_EX_END,
    };

    // Pretend this backup request arrived over MIDI by packaging it as inbound
    // SysEx7 UMP packets and feeding it back into SysExConf.
    return zlibs::utils::midi::write_sysex7_payload_as_ump_packets(
        zlibs::utils::midi::DEFAULT_RX_GROUP,
        std::span<const uint8_t>(&backup_request[1], sizeof(backup_request) - 2),
        [this](const midi_ump& packet)
        {
            _sysex_conf.handle_packet(packet);
            return true;
        });
}

bool System::find_next_backup_section(uint8_t& block, uint8_t& section) const
{
    while (block < _layout.blocks())
    {
        while (section < _layout.sections(block))
        {
            const bool skip_led_test_section =
                (block == static_cast<uint8_t>(sys::Config::Block::Leds)) &&
                ((section == static_cast<uint8_t>(sys::Config::Section::Leds::TestColor)) ||
                 (section == static_cast<uint8_t>(sys::Config::Section::Leds::TestBlink)));

            if (!skip_led_test_section)
            {
                return true;
            }

            section++;
        }

        block++;
        section = 0;
    }

    return false;
}

void System::finish_backup()
{
    LOG_INF("Finishing backup");

    _sysex_conf.set_user_error_ignore_mode(false);

    _backup_session.active = false;
    _backup_session.phase  = BackupPhase::Idle;
    _backup_restore_state  = BackupRestoreState::None;

    messaging::SystemSignal signal = {};
    signal.system_message          = messaging::SystemMessage::BackupEnd;
    messaging::publish(signal);

    io::Base::resume();

    if (_sysex_conf.is_configuration_enabled())
    {
        _sysex_conf_close_work.reschedule(SYSEX_CONFIGURATION_TIMEOUT_MS);
    }
}

void System::start_restore()
{
    LOG_INF("Starting restore");

    _sysex_conf_close_work.cancel();

    io::Base::freeze();
    _restore_queue.reset();
    _backup_restore_state = BackupRestoreState::Restore;
    _sysex_conf.set_user_error_ignore_mode(true);

    messaging::SystemSignal signal = {};
    signal.system_message          = messaging::SystemMessage::RestoreStart;
    messaging::publish(signal);
}

bool System::queue_restore_packet(const midi_ump& packet)
{
    if (!_restore_queue.insert(packet))
    {
        LOG_ERR("Restore queue overflow");
        return false;
    }

    _restore_work.reschedule(BACKUP_RESTORE_STEP_DELAY_MS);
    return true;
}

void System::run_restore_step()
{
    while (auto packet = _restore_queue.remove())
    {
        _sysex_conf.handle_packet(*packet);
    }
}

void System::finish_restore()
{
    LOG_INF("Finishing restore");

    _backup_restore_state = BackupRestoreState::None;

    messaging::SystemSignal signal = {};
    signal.system_message          = messaging::SystemMessage::RestoreEnd;
    messaging::publish(signal);

    schedule_reboot(fw_selector::FwType::Application);
}

void System::emit_preset_change(uint8_t preset)
{
    uint16_t preset_change_request[] = {
        static_cast<uint8_t>(zlibs::utils::sysex_conf::Wish::Set),
        static_cast<uint8_t>(zlibs::utils::sysex_conf::Amount::Single),
        static_cast<uint8_t>(sys::Config::Block::Global),
        static_cast<uint8_t>(sys::Config::Section::Global::SystemSettings),
        0x00,
        0x00,
        0x00,
        preset
    };

    _sysex_conf.send_custom_message(preset_change_request, false);
}

void System::schedule_forced_refresh(ForcedRefreshType type, uint32_t delay_ms)
{
    _forced_refresh_session.type = type;
    _forced_refresh_work.reschedule(delay_ms);
}

void System::start_forced_refresh()
{
    LOG_INF("Starting forced refresh");

    _forced_refresh_session.active          = true;
    _forced_refresh_session.io_index        = 0;
    _forced_refresh_session.component_index = 0;

    io::Base::freeze();

    messaging::ForcedRefreshStart forced_refresh_start = {};
    forced_refresh_start.type                          = _forced_refresh_session.type;
    messaging::publish(forced_refresh_start);

    messaging::SystemSignal burst_start = {};
    burst_start.system_message          = messaging::SystemMessage::BurstMidiStart;
    messaging::publish(burst_start);
}

void System::finish_forced_refresh()
{
    LOG_INF("Finishing forced refresh");

    messaging::SystemSignal burst_stop = {};
    burst_stop.system_message          = messaging::SystemMessage::BurstMidiStop;
    messaging::publish(burst_stop);

    messaging::ForcedRefreshStop forced_refresh_stop = {};
    forced_refresh_stop.type                         = _forced_refresh_session.type;
    messaging::publish(forced_refresh_stop);

    _forced_refresh_session = {};
    io::Base::resume();
}

void System::force_component_refresh()
{
    if ((_forced_refresh_session.type == ForcedRefreshType::PresetChange) &&
        _hwa.database().read(database::Config::Section::Common::CommonSettings,
                             Config::SystemSetting::DisableForcedRefreshAfterPresetChange))
    {
        if (_forced_refresh_session.active)
        {
            finish_forced_refresh();
        }

        LOG_INF("Forced refresh disabled for preset change");

        return;
    }

    // extra check here - it's possible that preset was changed and then backup/restore procedure started
    // in that case this would get called
    if (_backup_restore_state != BackupRestoreState::None)
    {
        if (_forced_refresh_session.active)
        {
            finish_forced_refresh();
        }

        return;
    }

    if (!_forced_refresh_session.active)
    {
        start_forced_refresh();
    }

    while (_forced_refresh_session.io_index < _hwa.io().size())
    {
        auto* component = _hwa.io().at(_forced_refresh_session.io_index);

        if (component == nullptr)
        {
            _forced_refresh_session.io_index++;
            _forced_refresh_session.component_index = 0;
            continue;
        }

        const auto total = component->refreshable_components();

        if ((total == 0) || (_forced_refresh_session.component_index >= total))
        {
            _forced_refresh_session.io_index++;
            _forced_refresh_session.component_index = 0;
            continue;
        }

        const auto count = std::min(FORCED_UPDATE_MAX_COMPONENTS_PER_RUN, total - _forced_refresh_session.component_index);

        component->force_refresh(_forced_refresh_session.component_index, count);
        _forced_refresh_session.component_index += count;

        if ((_forced_refresh_session.component_index < total) ||
            (_forced_refresh_session.io_index + 1 < _hwa.io().size()))
        {
            _forced_refresh_work.reschedule(FORCED_REFRESH_STAGE_DELAY_MS);
            return;
        }
    }

    finish_forced_refresh();
}

bool System::SysExDataHandler::send_response(const midi_ump& packet)
{
    return messaging::publish(messaging::UsbUmpBurstSignal{ .packet = packet });
}

zlibs::utils::sysex_conf::Status System::SysExDataHandler::custom_request(uint16_t request, CustomResponse& custom_response)
{
    auto result = zlibs::utils::sysex_conf::Status::Ack;

    static constexpr uint32_t UID_SHIFT_24  = 24;
    static constexpr uint32_t UID_SHIFT_16  = 16;
    static constexpr uint32_t UID_SHIFT_8   = 8;
    static constexpr uint32_t UID_BYTE_MASK = 0xFF;

    auto append_sw = [&custom_response]()
    {
        custom_response.append(OPENDECK_SW_VERSION_MAJOR);
        custom_response.append(OPENDECK_SW_VERSION_MINOR);
        custom_response.append(OPENDECK_SW_VERSION_REVISION);
    };

    auto append_hw = [&custom_response]()
    {
        custom_response.append((OPENDECK_TARGET_UID >> UID_SHIFT_24) & static_cast<uint32_t>(UID_BYTE_MASK));
        custom_response.append((OPENDECK_TARGET_UID >> UID_SHIFT_16) & static_cast<uint32_t>(UID_BYTE_MASK));
        custom_response.append((OPENDECK_TARGET_UID >> UID_SHIFT_8) & static_cast<uint32_t>(UID_BYTE_MASK));
        custom_response.append(OPENDECK_TARGET_UID & static_cast<uint32_t>(UID_BYTE_MASK));
    };

    switch (request)
    {
    case SYSEX_CR_FIRMWARE_VERSION:
    {
        append_sw();
    }
    break;

    case SYSEX_CR_HARDWARE_UID:
    {
        append_hw();
    }
    break;

    case SYSEX_CR_FIRMWARE_VERSION_HARDWARE_UID:
    {
        append_sw();
        append_hw();
    }
    break;

    case SYSEX_CR_FACTORY_RESET:
    {
        _system.schedule_factory_reset();
    }
    break;

    case SYSEX_CR_REBOOT_APP:
    {
        _system.schedule_reboot(fw_selector::FwType::Application);
    }
    break;

    case SYSEX_CR_REBOOT_BTLDR:
    {
        _system.schedule_reboot(fw_selector::FwType::Bootloader);
    }
    break;

    case SYSEX_CR_MAX_COMPONENTS:
    {
        custom_response.append(buttons::Collection::size());
        custom_response.append(encoders::Collection::size());
        custom_response.append(analog::Collection::size());
        custom_response.append(leds::Collection::size());
        custom_response.append(touchscreen::Collection::size());
    }
    break;

    case SYSEX_CR_SUPPORTED_PRESETS:
    {
        custom_response.append(_system._hwa.database().supported_presets());
    }
    break;

    case SYSEX_CR_BOOTLOADER_SUPPORT:
    {
        custom_response.append(1);
    }
    break;

    case SYSEX_CR_FULL_BACKUP:
    {
        if ((_system._backup_restore_state == BackupRestoreState::Backup) || _system._backup_session.active)
        {
            LOG_WRN("Ignoring duplicate backup request while a backup session is active");
            break;
        }

        _system.start_backup();
    }
    break;

    case SYSEX_CR_RESTORE_START:
    {
        _system.start_restore();
    }
    break;

    case SYSEX_CR_RESTORE_END:
    {
        _system.finish_restore();
    }
    break;

    default:
    {
        result = zlibs::utils::sysex_conf::Status::ErrorNotSupported;
    }
    break;
    }

    return result;
}

zlibs::utils::sysex_conf::Status System::SysExDataHandler::get(uint8_t   block,
                                                               uint8_t   section,
                                                               uint16_t  index,
                                                               uint16_t& value)
{
    return static_cast<zlibs::utils::sysex_conf::Status>(ConfigHandler.get(static_cast<sys::Config::Block>(block), section, index, value));
}

zlibs::utils::sysex_conf::Status System::SysExDataHandler::set(uint8_t  block,
                                                               uint8_t  section,
                                                               uint16_t index,
                                                               uint16_t value)
{
    return static_cast<zlibs::utils::sysex_conf::Status>(ConfigHandler.set(static_cast<sys::Config::Block>(block), section, index, value));
}

void System::DatabaseHandlers::preset_change([[maybe_unused]] uint8_t preset)
{
    if (!_system._components_initialized)
    {
        return;
    }

    messaging::SystemSignal signal = {};
    signal.system_message          = messaging::SystemMessage::PresetChanged;
    signal.value                   = preset;
    messaging::publish(signal);

    if (_system._backup_restore_state == BackupRestoreState::None)
    {
        LOG_INF("Preset changed, scheduling forced component refresh");
        _system.schedule_forced_refresh(ForcedRefreshType::PresetChange, PRESET_CHANGE_FORCED_REFRESH_DELAY);
    }
}

void System::DatabaseHandlers::initialized()
{
    // nothing to do here
}

void System::DatabaseHandlers::factory_reset_start()
{
    LOG_INF("Starting factory reset");

    messaging::SystemSignal signal = {};
    signal.system_message          = messaging::SystemMessage::FactoryResetStart;

    messaging::publish(signal);
}

void System::DatabaseHandlers::factory_reset_done()
{
    LOG_INF("Factory reset done, rebooting");

    messaging::SystemSignal signal = {};
    signal.system_message          = messaging::SystemMessage::FactoryResetEnd;

    messaging::publish(signal);

    _system.schedule_reboot(fw_selector::FwType::Application);
}

void System::schedule_factory_reset()
{
    LOG_INF("Scheduling factory reset");
    _factory_reset_work.reschedule(FACTORY_RESET_DELAY_MS);
}

void System::run_factory_reset()
{
    LOG_PANIC();
    LOG_INF("Running scheduled factory reset");

    if (_components_initialized)
    {
        io::Base::freeze();
        protocol::Base::freeze();
        collection_deinit();
    }

    if (!_hwa.database().factory_reset())
    {
        LOG_ERR("Factory reset failed");
    }
}

void System::schedule_reboot(fw_selector::FwType type)
{
    LOG_INF("Scheduling reboot to %s", type == fw_selector::FwType::Bootloader ? "bootloader" : "application");
    _reboot_type = type;
    _reboot_work.reschedule(REBOOT_DELAY_MS);
}

void System::update_sysex_configuration_session(bool was_open)
{
    const bool is_open = _sysex_conf.is_configuration_enabled();

    if (is_open)
    {
        if (_backup_restore_state != BackupRestoreState::None)
        {
            _sysex_conf_close_work.cancel();

            if (!was_open)
            {
                publish_configuration_session_state(messaging::SystemMessage::ConfigurationSessionOpened);
            }

            return;
        }

        _sysex_conf_close_work.reschedule(SYSEX_CONFIGURATION_TIMEOUT_MS);

        if (!was_open)
        {
            publish_configuration_session_state(messaging::SystemMessage::ConfigurationSessionOpened);
        }

        return;
    }

    _sysex_conf_close_work.cancel();

    if (was_open)
    {
        publish_configuration_session_state(messaging::SystemMessage::ConfigurationSessionClosed);
    }
}

void System::close_inactive_sysex_configuration_session()
{
    if (!_sysex_conf.is_configuration_enabled())
    {
        return;
    }

    if (_backup_restore_state != BackupRestoreState::None)
    {
        _sysex_conf_close_work.cancel();
        return;
    }

    LOG_INF("Closing inactive SysEx configuration session");

    _sysex_conf.close_connection();
    publish_configuration_session_state(messaging::SystemMessage::ConfigurationSessionClosed);
}

void System::publish_configuration_session_state(messaging::SystemMessage message)
{
    messaging::SystemSignal signal = {};
    signal.system_message          = message;

    messaging::publish(signal);
}

std::optional<uint8_t> System::sys_config_get(sys::Config::Section::Global section, size_t index, uint16_t& value)
{
    if (section != sys::Config::Section::Global::SystemSettings)
    {
        return {};
    }

    switch (index)
    {
    case static_cast<size_t>(sys::Config::SystemSetting::DisableForcedRefreshAfterPresetChange):
    case static_cast<size_t>(sys::Config::SystemSetting::EnablePresetChangeWithProgramChangeIn):
        break;

    default:
        return {};
    }

    uint32_t read_value = 0;

    auto result = _hwa.database().read(database::Config::Section::Common::CommonSettings, index, read_value)
                      ? sys::Config::Status::Ack
                      : sys::Config::Status::ErrorRead;

    value = read_value;

    return result;
}

std::optional<uint8_t> System::sys_config_set(sys::Config::Section::Global section, size_t index, uint16_t value)
{
    if (section != sys::Config::Section::Global::SystemSettings)
    {
        return {};
    }

    switch (index)
    {
    case static_cast<size_t>(sys::Config::SystemSetting::DisableForcedRefreshAfterPresetChange):
    case static_cast<size_t>(sys::Config::SystemSetting::EnablePresetChangeWithProgramChangeIn):
        break;

    default:
        return {};
    }

    auto result = _hwa.database().update(database::Config::Section::Common::CommonSettings, index, value)
                      ? sys::Config::Status::Ack
                      : sys::Config::Status::ErrorWrite;

    return result;
}
