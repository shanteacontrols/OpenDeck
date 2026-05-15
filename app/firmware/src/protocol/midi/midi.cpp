/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "firmware/src/protocol/midi/midi.h"
#include "firmware/src/system/config.h"
#include "firmware/src/util/conversion/conversion.h"
#include "firmware/src/signaling/signaling.h"
#include "firmware/src/util/configurable/configurable.h"
#include "firmware/src/global/midi_program.h"
#include "firmware/src/global/bpm.h"

#include <zephyr/logging/log.h>
#include <zephyr/kernel.h>

#include <span>

using namespace opendeck;
using namespace opendeck::protocol::midi;

namespace zmidi = zlibs::utils::midi;
namespace zmisc = zlibs::utils::misc;

namespace
{
    LOG_MODULE_REGISTER(midi, CONFIG_OPENDECK_LOG_LEVEL);    // NOLINT

    constexpr uint8_t MIDI_GROUP         = 0;
    constexpr uint8_t MIDI_CHANNEL_COUNT = 16;
    constexpr size_t  USB_BURST_MAX_SIZE = CONFIG_PROJECT_PROTOCOL_MIDI_USB_TX_QUEUE_SIZE / 2;

    uint8_t to_zero_based_channel(uint8_t channel)
    {
        return channel > 0 ? channel - 1 : 0;
    }

    size_t ump_size(const midi_ump& packet)
    {
        return UMP_NUM_WORDS(packet) * sizeof(packet.data[0]);
    }

}    // namespace

Midi::Midi(HwaUsb&    hwa_usb,
           HwaSerial& hwa_serial,
           HwaBle&    hwa_ble,
           Database&  database)
    : _hwa_usb(hwa_usb)
    , _hwa_serial(hwa_serial)
    , _hwa_ble(hwa_ble)
    , _database(database)
    , _clock_timer(
          zmisc::Timer::Type::Repeating,
          [this]()
          {
              _serial.send(zmidi::midi1::timing_clock(MIDI_GROUP));
          })
    , _thread([this]()
              {
                  read_loop();
              },
              [this]()
              {
                  for (auto& event : _poll_events)
                  {
                      k_poll_signal_raise(event.signal, 0);
                  }
              })
{
    _hwa_usb.register_on_ready_handler([]()
                                       {
                                           signaling::SystemSignal signal = {};
                                           signal.system_event            = signaling::SystemEvent::UsbMidiReady;
                                           signaling::publish(signal);
                                       });

    // place all interfaces in array for easier access
    _midi_interface[static_cast<size_t>(Interface::Usb)]    = &_usb;
    _midi_interface[static_cast<size_t>(Interface::Serial)] = &_serial;
    _midi_interface[static_cast<size_t>(Interface::Ble)]    = &_ble;

    k_poll_event_init(
        &_poll_events[static_cast<size_t>(Interface::Usb)],
        K_POLL_TYPE_SIGNAL,
        K_POLL_MODE_NOTIFY_ONLY,
        _hwa_usb.data_available_signal());
    k_poll_event_init(
        &_poll_events[static_cast<size_t>(Interface::Serial)],
        K_POLL_TYPE_SIGNAL,
        K_POLL_MODE_NOTIFY_ONLY,
        _hwa_serial.data_available_signal());
    k_poll_event_init(
        &_poll_events[static_cast<size_t>(Interface::Ble)],
        K_POLL_TYPE_SIGNAL,
        K_POLL_MODE_NOTIFY_ONLY,
        _hwa_ble.data_available_signal());

    signaling::subscribe<signaling::SystemSignal>(
        [this](const signaling::SystemSignal& event)
        {
            switch (event.system_event)
            {
            case signaling::SystemEvent::PresetChanged:
            {
                init();
            }
            break;

            case signaling::SystemEvent::MidiBpmChange:
            {
                if (is_setting_enabled(Setting::DinEnabled) && is_setting_enabled(Setting::SendMidiClockDin))
                {
                    _clock_timer.start(Bpm.bpm_to_usec(Bpm.value()), zmisc::TimerUnit::Us);
                }
            }
            break;

            case signaling::SystemEvent::BurstMidiStart:
            {
                _burst_midi_active = true;
                _usb_burst_count   = 0;
                _usb_burst_size    = 0;
            }
            break;

            case signaling::SystemEvent::BurstMidiStop:
            {
                flush_usb_burst();
                _burst_midi_active = false;
            }
            break;

            default:
                break;
            }
        });

    signaling::subscribe<signaling::MidiIoSignal>(
        [this](const signaling::MidiIoSignal& event)
        {
            send(event);
        });

    signaling::subscribe<signaling::UmpSignal>(
        [this](const signaling::UmpSignal& event)
        {
            if (event.direction != signaling::SignalDirection::Out)
            {
                return;
            }

            send(event);
        });

    signaling::subscribe<signaling::UsbUmpBurstSignal>(
        [this](const signaling::UsbUmpBurstSignal& event)
        {
            append_usb_burst_packet(event.packet);

            if (_burst_midi_active)
            {
                return;
            }

            if (!zmidi::is_sysex7_packet(event.packet))
            {
                flush_usb_burst();
                return;
            }

            if (zmidi::sysex7_ends_message(zmidi::sysex7_status(event.packet)))
            {
                flush_usb_burst();
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

Midi::~Midi()
{
    [[maybe_unused]] auto ret = shutdown();
}

bool Midi::init()
{
    _standard_note_off = is_setting_enabled(Setting::StandardNoteOff);

    if (!setup_usb())
    {
        return false;
    }

    if (!setup_serial())
    {
        return false;
    }

    if (!setup_ble())
    {
        return false;
    }

    if (!setup_thru())
    {
        return false;
    }

    _initialized = true;
    _thread.run();

    return true;
}

bool Midi::deinit()
{
    return shutdown();
}

bool Midi::shutdown()
{
    _initialized = false;
    _thread.destroy();
    flush_usb_burst();

    if (!_serial.deinit())
    {
        return false;
    }

    if (!_usb.deinit())
    {
        return false;
    }

    if (!_ble.deinit())
    {
        return false;
    }

    _din_loopback = false;

    return true;
}

bool Midi::append_usb_burst_packet(const midi_ump& packet)
{
    const auto packet_size = ump_size(packet);

    if ((_usb_burst_count == _usb_burst_packets.size()) ||
        ((_usb_burst_size + packet_size) > USB_BURST_MAX_SIZE))
    {
        if (!flush_usb_burst())
        {
            return false;
        }
    }

    _usb_burst_packets[_usb_burst_count++] = packet;
    _usb_burst_size += packet_size;
    return true;
}

bool Midi::flush_usb_burst()
{
    if (_usb_burst_count == 0)
    {
        return true;
    }

    const auto written = _hwa_usb.write(std::span<const midi_ump>(_usb_burst_packets.data(), _usb_burst_count));
    _usb_burst_count   = 0;
    _usb_burst_size    = 0;
    return written;
}

bool Midi::setup_usb()
{
    if (_usb.initialized())
    {
        return true;
    }

    if (!_usb.init())
    {
        return false;
    }

    return true;
}
bool Midi::setup_serial()
{
    const auto is_initialized = _serial.initialized();
    const bool din_enabled    = is_setting_enabled(Setting::DinEnabled);

    if (din_enabled)
    {
        if (!is_initialized)
        {
            _serial.init();
        }

        if (!apply_din_loopback())
        {
            return false;
        }
    }
    else
    {
        if (is_initialized)
        {
            _serial.deinit();
        }
        _din_loopback = false;
    }

    if (is_setting_enabled(Setting::SendMidiClockDin))
    {
        _clock_timer.start(Bpm.bpm_to_usec(Bpm.value()), zmisc::TimerUnit::Us);
    }
    else
    {
        _clock_timer.stop();
    }

    return true;
}

bool Midi::setup_ble()
{
    if (is_setting_enabled(Setting::BleEnabled))
    {
        if (!_ble.initialized())
        {
            _ble.init();
        }
    }
    else
    {
        if (_ble.initialized())
        {
            _ble.deinit();
        }
    }

    return true;
}

bool Midi::setup_thru()
{
    if (is_setting_enabled(Setting::DinThruDin))
    {
        _serial.register_thru_interface(_serial.thru_interface());
    }
    else
    {
        _serial.unregister_thru_interface(_serial.thru_interface());
    }

    if (is_setting_enabled(Setting::DinThruUsb))
    {
        _serial.register_thru_interface(_usb.thru_interface());
    }
    else
    {
        _serial.unregister_thru_interface(_usb.thru_interface());
    }

    if (is_setting_enabled(Setting::DinThruBle))
    {
        _serial.register_thru_interface(_ble.thru_interface());
    }
    else
    {
        _serial.unregister_thru_interface(_ble.thru_interface());
    }

    if (is_setting_enabled(Setting::UsbThruDin))
    {
        _usb.register_thru_interface(_serial.thru_interface());
    }
    else
    {
        _usb.unregister_thru_interface(_serial.thru_interface());
    }

    if (is_setting_enabled(Setting::UsbThruUsb))
    {
        _usb.register_thru_interface(_usb.thru_interface());
    }
    else
    {
        _usb.unregister_thru_interface(_usb.thru_interface());
    }

    if (is_setting_enabled(Setting::UsbThruBle))
    {
        _usb.register_thru_interface(_ble.thru_interface());
    }
    else
    {
        _usb.unregister_thru_interface(_ble.thru_interface());
    }

    if (is_setting_enabled(Setting::BleThruDin))
    {
        _ble.register_thru_interface(_serial.thru_interface());
    }
    else
    {
        _ble.unregister_thru_interface(_serial.thru_interface());
    }

    if (is_setting_enabled(Setting::BleThruUsb))
    {
        _ble.register_thru_interface(_usb.thru_interface());
    }
    else
    {
        _ble.unregister_thru_interface(_usb.thru_interface());
    }

    if (is_setting_enabled(Setting::BleThruBle))
    {
        _ble.register_thru_interface(_ble.thru_interface());
    }
    else
    {
        _ble.unregister_thru_interface(_ble.thru_interface());
    }

    return true;
}

void Midi::read_loop()
{
    while (true)
    {
        protocol::Base::wait_until_running();

        for (auto& event : _poll_events)
        {
            event.state = K_POLL_STATE_NOT_READY;
        }

        [[maybe_unused]] auto ret = k_poll(_poll_events.data(), static_cast<int>(_poll_events.size()), K_FOREVER);

        if (!_initialized)
        {
            return;
        }

        if (protocol::Base::is_frozen())
        {
            continue;
        }

        for (size_t i = 0; i < _poll_events.size(); i++)
        {
            if (_poll_events[i].state != K_POLL_STATE_SIGNALED)
            {
                continue;
            }

            k_poll_signal_reset(_poll_events[i].signal);
            read_interface(i);
        }
    }
}

void Midi::read_interface(size_t interface_index)
{
    auto interface_instance = _midi_interface[interface_index];

    if (!interface_instance->initialized())
    {
        return;
    }

    while (true)
    {
        const auto packet = interface_instance->read();

        if (!packet.has_value())
        {
            break;
        }

        signaling::publish(signaling::TrafficSignal{
            .transport = interface_to_transport(interface_index),
            .direction = signaling::SignalDirection::In,
        });

        const auto message = decode_message(packet.value());

        switch (message.type)
        {
        case MessageType::SysEx:
        {
            // process sysex messages only from usb interface
            if (interface_index != static_cast<size_t>(Interface::Usb))
            {
                continue;
            }
        }
        break;

        case MessageType::ProgramChange:
        {
            MidiProgram.set_program(message.channel, message.data1);
        }
        break;

        default:
            break;
        }

        signaling::publish(signaling::UmpSignal{
            .direction = signaling::SignalDirection::In,
            .packet    = packet.value(),
        });
    }
}

bool Midi::is_setting_enabled(Setting feature)
{
    return _database.read(database::Config::Section::Global::MidiSettings, feature);
}

bool Midi::is_din_loopback_required()
{
    return (is_setting_enabled(Setting::DinEnabled) &&
            is_setting_enabled(Setting::DinThruDin) &&
            !is_setting_enabled(Setting::DinThruUsb) &&
            !is_setting_enabled(Setting::DinThruBle));
}

bool Midi::apply_din_loopback()
{
    if (!is_setting_enabled(Setting::DinEnabled))
    {
        _din_loopback = false;
        return true;
    }

    const bool desired = is_din_loopback_required();

    if (_din_loopback == desired)
    {
        return true;
    }

    if (!_hwa_serial.set_loopback(desired))
    {
        return false;
    }

    _din_loopback = desired;
    return true;
}

void Midi::send(const signaling::MidiIoSignal& event)
{
    if (protocol::Base::is_frozen())
    {
        return;
    }

    if (event.source == signaling::IoEventSource::AnalogSwitch)
    {
        return;
    }

    const uint8_t global_channel = _database.read(database::Config::Section::Global::MidiSettings, Setting::GlobalChannel);
    const uint8_t channel        = _database.read(database::Config::Section::Global::MidiSettings,
                                                  Setting::UseGlobalChannel)
                                       ? global_channel
                                       : event.channel;
    const bool    use_omni       = channel == OMNI_CHANNEL;

    auto send_ump = [this](const midi_ump& packet)
    {
        if (_burst_midi_active)
        {
            (void)signaling::publish(signaling::UsbUmpBurstSignal{ .packet = packet });

            signaling::UmpSignal non_usb_signal = {};
            non_usb_signal.packet               = packet;

            non_usb_signal.route = signaling::UmpSignal::Route::Din;
            send(non_usb_signal);

            non_usb_signal.route = signaling::UmpSignal::Route::Ble;
            send(non_usb_signal);
            return;
        }

        signaling::UmpSignal signal = {};
        signal.packet               = packet;
        send(signal);
    };

    auto send_channel_voice = [&](auto builder)
    {
        if (use_omni)
        {
            for (uint8_t channel = 1; channel <= MIDI_CHANNEL_COUNT; channel++)
            {
                send_ump(builder(channel));
            }
        }
        else
        {
            send_ump(builder(channel));
        }
    };

    switch (event.message)
    {
    case MessageType::NoteOff:
    {
        send_channel_voice(
            [&](uint8_t channel)
            {
                return zmidi::midi1::note_off(
                    MIDI_GROUP,
                    to_zero_based_channel(channel),
                    event.index,
                    event.value,
                    !_standard_note_off);
            });
    }
    break;

    case MessageType::NoteOn:
    {
        send_channel_voice(
            [&](uint8_t channel)
            {
                return zmidi::midi1::note_on(
                    MIDI_GROUP,
                    to_zero_based_channel(channel),
                    event.index,
                    event.value);
            });
    }
    break;

    case MessageType::ControlChange:
    {
        send_channel_voice(
            [&](uint8_t channel)
            {
                return zmidi::midi1::control_change(
                    MIDI_GROUP,
                    to_zero_based_channel(channel),
                    event.index,
                    event.value);
            });
    }
    break;

    case MessageType::ProgramChange:
    {
        send_channel_voice(
            [&](uint8_t channel)
            {
                return zmidi::midi1::program_change(
                    MIDI_GROUP,
                    to_zero_based_channel(channel),
                    event.index);
            });
    }
    break;

    case MessageType::SysRealTimeClock:
    {
        send_ump(zmidi::midi1::timing_clock(MIDI_GROUP));
    }
    break;

    case MessageType::SysRealTimeStart:
    {
        send_ump(zmidi::midi1::start(MIDI_GROUP));
    }
    break;

    case MessageType::SysRealTimeContinue:
    {
        send_ump(zmidi::midi1::continue_playback(MIDI_GROUP));
    }
    break;

    case MessageType::SysRealTimeStop:
    {
        send_ump(zmidi::midi1::stop(MIDI_GROUP));
    }
    break;

    case MessageType::SysRealTimeActiveSensing:
    {
        send_ump(zmidi::midi1::active_sensing(MIDI_GROUP));
    }
    break;

    case MessageType::SysRealTimeSystemReset:
    {
        send_ump(zmidi::midi1::system_reset(MIDI_GROUP));
    }
    break;

    case MessageType::MmcPlay:
    case MessageType::MmcStop:
    case MessageType::MmcPause:
    case MessageType::MmcRecordStart:
    case MessageType::MmcRecordStop:
    {
        std::optional<midi_ump> packet = std::nullopt;

        switch (event.message)
        {
        case MessageType::MmcPlay:
        {
            packet = zmidi::midi1::mmc<zmidi::MessageType::MmcPlay>(MIDI_GROUP, event.index);
        }
        break;

        case MessageType::MmcStop:
        {
            packet = zmidi::midi1::mmc<zmidi::MessageType::MmcStop>(MIDI_GROUP, event.index);
        }
        break;

        case MessageType::MmcPause:
        {
            packet = zmidi::midi1::mmc<zmidi::MessageType::MmcPause>(MIDI_GROUP, event.index);
        }
        break;

        case MessageType::MmcRecordStart:
        {
            packet = zmidi::midi1::mmc<zmidi::MessageType::MmcRecordStart>(MIDI_GROUP, event.index);
        }
        break;

        case MessageType::MmcRecordStop:
        {
            packet = zmidi::midi1::mmc<zmidi::MessageType::MmcRecordStop>(MIDI_GROUP, event.index);
        }
        break;

        default:
            break;
        }

        if (packet.has_value())
        {
            send_ump(packet.value());
        }
    }
    break;

    default:
        break;
    }
}

void Midi::send(const signaling::UmpSignal& event)
{
    if (protocol::Base::is_frozen())
    {
        return;
    }

    for (size_t i = 0; i < _midi_interface.size(); i++)
    {
        auto*      interface_instance = _midi_interface[i];
        const auto transport          = interface_to_transport(i);

        if ((interface_instance != nullptr) && interface_instance->initialized())
        {
            if (event.direction == signaling::SignalDirection::Out)
            {
                switch (event.route)
                {
                case signaling::UmpSignal::Route::All:
                    break;

                case signaling::UmpSignal::Route::Usb:
                {
                    if (transport != signaling::TrafficTransport::Usb)
                    {
                        continue;
                    }
                }
                break;

                case signaling::UmpSignal::Route::Din:
                {
                    if (transport != signaling::TrafficTransport::Din)
                    {
                        continue;
                    }
                }
                break;

                case signaling::UmpSignal::Route::Ble:
                {
                    if (transport != signaling::TrafficTransport::Ble)
                    {
                        continue;
                    }
                }
                break;
                }
            }

            if ((transport == signaling::TrafficTransport::Ble) && !_hwa_ble.ready())
            {
                continue;
            }

            if ((transport == signaling::TrafficTransport::Usb) && !_hwa_usb.ready())
            {
                continue;
            }

            if (!interface_instance->send(event.packet))
            {
                LOG_ERR("Ump TX failed on interface %d (transport=%d, mt=%d)",
                        static_cast<int>(i),
                        static_cast<int>(transport),
                        static_cast<int>(UMP_MT(event.packet)));
            }

            signaling::publish(signaling::TrafficSignal{
                .transport = transport,
                .direction = signaling::SignalDirection::Out,
            });
        }
    }
}

signaling::TrafficTransport Midi::interface_to_transport(size_t index) const
{
    switch (index)
    {
    case static_cast<size_t>(Interface::Usb):
        return signaling::TrafficTransport::Usb;

    case static_cast<size_t>(Interface::Serial):
        return signaling::TrafficTransport::Din;

    case static_cast<size_t>(Interface::Ble):
        return signaling::TrafficTransport::Ble;

    default:
        return signaling::TrafficTransport::Usb;
    }
}

void Midi::set_note_off_mode(NoteOffType type)
{
    _standard_note_off = type == NoteOffType::StandardNoteOff;
}

std::optional<uint8_t> Midi::sys_config_get(sys::Config::Section::Global section, size_t index, uint16_t& value)
{
    if (section != sys::Config::Section::Global::MidiSettings)
    {
        return {};
    }

    [[maybe_unused]] uint32_t read_value = 0;
    [[maybe_unused]] uint8_t  result     = sys::Config::Status::ErrorRead;

    auto feature = static_cast<Setting>(index);

    switch (feature)
    {
    case Setting::StandardNoteOff:
    {
        result = _database.read(util::Conversion::sys_2_db_section(section), index, read_value)
                     ? sys::Config::Status::Ack
                     : sys::Config::Status::ErrorRead;
    }
    break;

    case Setting::RunningStatus:
    case Setting::DinEnabled:
    case Setting::DinThruDin:
    case Setting::DinThruUsb:
    case Setting::UsbThruDin:
    {
        if (_hwa_serial.supported())
        {
            if (!is_setting_enabled(Setting::DinEnabled) && _hwa_serial.allocated(io::common::Allocatable::Interface::Uart))
            {
                result = sys::Config::Status::SerialPeripheralAllocatedError;
            }
            else
            {
                result = _database.read(util::Conversion::sys_2_db_section(section), index, read_value)
                             ? sys::Config::Status::Ack
                             : sys::Config::Status::ErrorRead;
            }
        }
        else
        {
            result = sys::Config::Status::ErrorNotSupported;
        }
    }
    break;

    case Setting::BleEnabled:
    case Setting::BleThruUsb:
    case Setting::BleThruBle:
    case Setting::UsbThruBle:
    {
        if (_hwa_ble.supported())
        {
            result = _database.read(util::Conversion::sys_2_db_section(section), index, read_value)
                         ? sys::Config::Status::Ack
                         : sys::Config::Status::ErrorRead;
        }
        else
        {
            result = sys::Config::Status::ErrorNotSupported;
        }
    }
    break;

    case Setting::DinThruBle:
    case Setting::BleThruDin:
    {
        if (_hwa_serial.supported())
        {
            if (!is_setting_enabled(Setting::DinEnabled) && _hwa_serial.allocated(io::common::Allocatable::Interface::Uart))
            {
                result = sys::Config::Status::SerialPeripheralAllocatedError;
            }
            else
            {
                if (_hwa_ble.supported())
                {
                    result = _database.read(util::Conversion::sys_2_db_section(section), index, read_value)
                                 ? sys::Config::Status::Ack
                                 : sys::Config::Status::ErrorRead;
                }
                else
                {
                    result = sys::Config::Status::ErrorNotSupported;
                }
            }
        }
        else
        {
            result = sys::Config::Status::ErrorNotSupported;
        }
    }
    break;

    default:
    {
        result = _database.read(util::Conversion::sys_2_db_section(section), index, read_value)
                     ? sys::Config::Status::Ack
                     : sys::Config::Status::ErrorRead;
    }
    break;
    }

    value = read_value;
    return result;
}

std::optional<uint8_t> Midi::sys_config_set(sys::Config::Section::Global section, size_t index, uint16_t value)
{
    if (section != sys::Config::Section::Global::MidiSettings)
    {
        return {};
    }

    [[maybe_unused]] uint8_t result               = sys::Config::Status::ErrorWrite;
    [[maybe_unused]] bool    write_to_db          = true;
    [[maybe_unused]] auto    din_midi_init_action = io::common::InitAction::AsIs;
    [[maybe_unused]] auto    ble_midi_init_action = io::common::InitAction::AsIs;
    [[maybe_unused]] bool    check_din_loopback   = false;

    auto setting = static_cast<Setting>(index);

    switch (setting)
    {
    case Setting::RunningStatus:
    {
        // this setting applies to din midi only
        if (_hwa_serial.supported())
        {
            if (!is_setting_enabled(Setting::DinEnabled) && _hwa_serial.allocated(io::common::Allocatable::Interface::Uart))
            {
                result = sys::Config::Status::SerialPeripheralAllocatedError;
            }
            else
            {
                // Running status handling is internal to the zlibs serial parser.
                result = sys::Config::Status::Ack;
            }
        }
        else
        {
            result = sys::Config::Status::ErrorNotSupported;
        }
    }
    break;

    case Setting::StandardNoteOff:
    {
        set_note_off_mode(value ? NoteOffType::StandardNoteOff : NoteOffType::NoteOnZeroVel);
        result = sys::Config::Status::Ack;
    }
    break;

    case Setting::DinEnabled:
    {
        if (_hwa_serial.supported())
        {
            if (!is_setting_enabled(Setting::DinEnabled) && _hwa_serial.allocated(io::common::Allocatable::Interface::Uart))
            {
                result = sys::Config::Status::SerialPeripheralAllocatedError;
            }
            else
            {
                if (value)
                {
                    din_midi_init_action = io::common::InitAction::Init;
                }
                else
                {
                    din_midi_init_action = io::common::InitAction::DeInit;
                }

                check_din_loopback = true;
                result             = sys::Config::Status::Ack;
            }
        }
        else
        {
            result = sys::Config::Status::ErrorNotSupported;
        }
    }
    break;

    case Setting::BleEnabled:
    {
        if (_hwa_ble.supported())
        {
            if (value)
            {
                ble_midi_init_action = io::common::InitAction::Init;
            }
            else
            {
                ble_midi_init_action = io::common::InitAction::DeInit;
            }

            result = sys::Config::Status::Ack;
        }
        else
        {
            result = sys::Config::Status::ErrorNotSupported;
        }
    }
    break;

    case Setting::DinThruDin:
    {
        if (_hwa_serial.supported())
        {
            if (value)
            {
                _serial.register_thru_interface(_serial.thru_interface());
            }
            else
            {
                _serial.unregister_thru_interface(_serial.thru_interface());
            }

            result             = sys::Config::Status::Ack;
            check_din_loopback = true;
        }
        else
        {
            result = sys::Config::Status::ErrorNotSupported;
        }
    }
    break;

    case Setting::DinThruUsb:
    {
        if (_hwa_serial.supported())
        {
            if (value)
            {
                _serial.register_thru_interface(_usb.thru_interface());
            }
            else
            {
                _serial.unregister_thru_interface(_usb.thru_interface());
            }

            result             = sys::Config::Status::Ack;
            check_din_loopback = true;
        }
        else
        {
            result = sys::Config::Status::ErrorNotSupported;
        }
    }
    break;

    case Setting::DinThruBle:
    {
        if (_hwa_serial.supported())
        {
            if (!is_setting_enabled(Setting::DinEnabled) && _hwa_serial.allocated(io::common::Allocatable::Interface::Uart))
            {
                result = sys::Config::Status::SerialPeripheralAllocatedError;
            }
            else
            {
                if (_hwa_ble.supported())
                {
                    if (value)
                    {
                        _serial.register_thru_interface(_ble.thru_interface());
                    }
                    else
                    {
                        _serial.unregister_thru_interface(_ble.thru_interface());
                    }

                    result             = sys::Config::Status::Ack;
                    check_din_loopback = true;
                }
                else
                {
                    result = sys::Config::Status::ErrorNotSupported;
                }
            }
        }
        else
        {
            result = sys::Config::Status::ErrorNotSupported;
        }
    }
    break;

    case Setting::UsbThruDin:
    {
        if (_hwa_serial.supported())
        {
            if (value)
            {
                _usb.register_thru_interface(_serial.thru_interface());
            }
            else
            {
                _usb.unregister_thru_interface(_serial.thru_interface());
            }

            result = sys::Config::Status::Ack;
        }
        else
        {
            result = sys::Config::Status::ErrorNotSupported;
        }
    }
    break;

    case Setting::UsbThruUsb:
    {
        if (value)
        {
            _usb.register_thru_interface(_usb.thru_interface());
        }
        else
        {
            _usb.unregister_thru_interface(_usb.thru_interface());
        }

        result = sys::Config::Status::Ack;
    }
    break;

    case Setting::UsbThruBle:
    {
        if (_hwa_ble.supported())
        {
            if (value)
            {
                _usb.register_thru_interface(_ble.thru_interface());
            }
            else
            {
                _usb.unregister_thru_interface(_ble.thru_interface());
            }

            result = sys::Config::Status::Ack;
        }
        else
        {
            result = sys::Config::Status::ErrorNotSupported;
        }
    }
    break;

    case Setting::BleThruDin:
    {
        if (_hwa_serial.supported())
        {
            if (!is_setting_enabled(Setting::DinEnabled) && _hwa_serial.allocated(io::common::Allocatable::Interface::Uart))
            {
                result = sys::Config::Status::SerialPeripheralAllocatedError;
            }
            else
            {
                if (_hwa_ble.supported())
                {
                    if (value)
                    {
                        _ble.register_thru_interface(_serial.thru_interface());
                    }
                    else
                    {
                        _ble.unregister_thru_interface(_serial.thru_interface());
                    }

                    result = sys::Config::Status::Ack;
                }
                else
                {
                    result = sys::Config::Status::ErrorNotSupported;
                }
            }
        }
        else
        {
            result = sys::Config::Status::ErrorNotSupported;
        }
    }
    break;

    case Setting::BleThruUsb:
    {
        if (_hwa_ble.supported())
        {
            if (value)
            {
                _ble.register_thru_interface(_usb.thru_interface());
            }
            else
            {
                _ble.unregister_thru_interface(_usb.thru_interface());
            }

            result = sys::Config::Status::Ack;
        }
        else
        {
            result = sys::Config::Status::ErrorNotSupported;
        }
    }
    break;

    case Setting::BleThruBle:
    {
        if (_hwa_ble.supported())
        {
            if (value)
            {
                _ble.register_thru_interface(_ble.thru_interface());
            }
            else
            {
                _ble.unregister_thru_interface(_ble.thru_interface());
            }

            result = sys::Config::Status::Ack;
        }
        else
        {
            result = sys::Config::Status::ErrorNotSupported;
        }
    }
    break;

    case Setting::GlobalChannel:
    {
        if ((value < 1) || (value > OMNI_CHANNEL))
        {
            // invalid channel
            result = sys::Config::Status::ErrorNewValue;
            break;
        }

        result = sys::Config::Status::Ack;
    }
    break;

    case Setting::SendMidiClockDin:
    {
        if (value)
        {
            _clock_timer.start(Bpm.bpm_to_usec(Bpm.value()), zmisc::TimerUnit::Us);
        }
        else
        {
            _clock_timer.stop();
        }
        result = sys::Config::Status::Ack;
    }
    break;

    default:
    {
        result = sys::Config::Status::Ack;
    }
    break;
    }

    if ((result == sys::Config::Status::Ack) && write_to_db)
    {
        result = _database.update(util::Conversion::sys_2_db_section(section), index, value)
                     ? sys::Config::Status::Ack
                     : sys::Config::Status::ErrorWrite;

        switch (din_midi_init_action)
        {
        case io::common::InitAction::Init:
        {
            _serial.init();

            if (is_setting_enabled(Setting::SendMidiClockDin))
            {
                _clock_timer.start(Bpm.bpm_to_usec(Bpm.value()), zmisc::TimerUnit::Us);
            }
        }
        break;

        case io::common::InitAction::DeInit:
        {
            if (is_setting_enabled(Setting::SendMidiClockDin))
            {
                _clock_timer.stop();
            }

            _serial.deinit();
        }
        break;

        default:
            break;
        }

        switch (ble_midi_init_action)
        {
        case io::common::InitAction::Init:
        {
            _ble.init();
        }
        break;

        case io::common::InitAction::DeInit:
        {
            _ble.deinit();
        }
        break;

        default:
            break;
        }
    }

    // no need to check this if init/deinit has been already called for DIN
    if (result && check_din_loopback && din_midi_init_action == io::common::InitAction::AsIs)
    {
        // Special consideration for DIN MIDI:
        // To make DIN to DIN thruing as fast as possible,
        // if certain criteria matches (DIN enabled, just DIN to DIN
        // thru enabled for DIN interface), enable BSP loopback:
        // this will ensure that incoming serial data is transmitted
        // to output port as soon as it is received in the BSP layer,
        // without even storing incoming data to internal buffers. This
        // will also ensure that nothing will be read from serial via
        // HWA interface.
        result = apply_din_loopback();
    }

    return result;
}
