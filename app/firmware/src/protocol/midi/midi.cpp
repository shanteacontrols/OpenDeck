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

Midi::Midi(UsbMidi&    usb,
           SerialMidi& serial,
           BleMidi&    ble,
           Database&   database)
    : _usb(usb)
    , _serial(serial)
    , _ble(ble)
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
                  OpenDeckTransport::wake_all(_interfaces);
              })
{
    // place all interfaces in array for easier access
    _interfaces[static_cast<size_t>(Interface::Usb)]    = &_usb;
    _interfaces[static_cast<size_t>(Interface::Serial)] = &_serial;
    _interfaces[static_cast<size_t>(Interface::Ble)]    = &_ble;

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

    const auto written = _usb.write(std::span<const midi_ump>(_usb_burst_packets.data(), _usb_burst_count));
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
    }
    else
    {
        if (is_initialized)
        {
            _serial.deinit();
        }
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
    auto route = [](zmidi::Base& source, zmidi::Thru& destination, bool enabled)
    {
        source.unregister_thru_interface(destination);

        if (!enabled)
        {
            return true;
        }

        return source.register_thru_interface(destination);
    };

    bool result = true;

    result = route(_serial, _serial.thru_interface(), is_setting_enabled(Setting::DinThruDin)) && result;
    result = route(_serial, _usb.thru_interface(), is_setting_enabled(Setting::DinThruUsb)) && result;
    result = route(_serial, _ble.thru_interface(), is_setting_enabled(Setting::DinThruBle)) && result;
    result = route(_usb, _serial.thru_interface(), is_setting_enabled(Setting::UsbThruDin)) && result;
    result = route(_usb, _usb.thru_interface(), is_setting_enabled(Setting::UsbThruUsb)) && result;
    result = route(_usb, _ble.thru_interface(), is_setting_enabled(Setting::UsbThruBle)) && result;
    result = route(_ble, _serial.thru_interface(), is_setting_enabled(Setting::BleThruDin)) && result;
    result = route(_ble, _usb.thru_interface(), is_setting_enabled(Setting::BleThruUsb)) && result;
    result = route(_ble, _ble.thru_interface(), is_setting_enabled(Setting::BleThruBle)) && result;

    return result;
}

void Midi::read_loop()
{
    while (true)
    {
        protocol::Base::wait_until_running();

        const auto interface_index = OpenDeckTransport::wait_for_data(_interfaces);

        if (!_initialized)
        {
            return;
        }

        if (protocol::Base::is_frozen())
        {
            continue;
        }

        if (!interface_index.has_value())
        {
            continue;
        }

        read_interface(interface_index.value());
    }
}

void Midi::read_interface(size_t interface_index)
{
    auto interface_instance = _interfaces[interface_index];

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

    for (size_t i = 0; i < _interfaces.size(); i++)
    {
        auto*      interface_instance = _interfaces[i];
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

            if ((transport == signaling::TrafficTransport::Ble) && !_ble.ready())
            {
                continue;
            }

            if ((transport == signaling::TrafficTransport::Usb) && !_usb.ready())
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

    auto read_midi_setting = [&]()
    {
        return _database.read(util::Conversion::sys_2_db_section(section), index, read_value)
                   ? sys::Config::Status::Ack
                   : sys::Config::Status::ErrorRead;
    };

    switch (feature)
    {
    case Setting::StandardNoteOff:
    {
        result = read_midi_setting();
    }
    break;

    case Setting::RunningStatus:
    case Setting::DinEnabled:
    case Setting::DinThruDin:
    {
        if (_serial.supported())
        {
            if (!is_setting_enabled(Setting::DinEnabled) && _serial.allocated(io::common::Allocatable::Interface::Uart))
            {
                result = sys::Config::Status::SerialPeripheralAllocatedError;
            }
            else
            {
                result = read_midi_setting();
            }
        }
        else
        {
            result = sys::Config::Status::ErrorNotSupported;
        }
    }
    break;

    case Setting::DinThruUsb:
    case Setting::UsbThruDin:
    {
        if (_usb.supported() && _serial.supported())
        {
            if (!is_setting_enabled(Setting::DinEnabled) && _serial.allocated(io::common::Allocatable::Interface::Uart))
            {
                result = sys::Config::Status::SerialPeripheralAllocatedError;
            }
            else
            {
                result = read_midi_setting();
            }
        }
        else
        {
            result = sys::Config::Status::ErrorNotSupported;
        }
    }
    break;

    case Setting::BleEnabled:
    case Setting::BleThruBle:
    {
        if (_ble.supported())
        {
            result = read_midi_setting();
        }
        else
        {
            result = sys::Config::Status::ErrorNotSupported;
        }
    }
    break;

    case Setting::UsbThruBle:
    case Setting::BleThruUsb:
    {
        if (_usb.supported() && _ble.supported())
        {
            result = read_midi_setting();
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
        if (_serial.supported())
        {
            if (!is_setting_enabled(Setting::DinEnabled) && _serial.allocated(io::common::Allocatable::Interface::Uart))
            {
                result = sys::Config::Status::SerialPeripheralAllocatedError;
            }
            else
            {
                if (_ble.supported())
                {
                    result = read_midi_setting();
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

    case Setting::UsbThruUsb:
    {
        result = _usb.supported() ? read_midi_setting() : sys::Config::Status::ErrorNotSupported;
    }
    break;

    default:
    {
        result = read_midi_setting();
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
    [[maybe_unused]] bool    refresh_thru         = false;

    auto setting = static_cast<Setting>(index);

    auto din_available = [&]()
    {
        if (!_serial.supported())
        {
            return sys::Config::Status::ErrorNotSupported;
        }

        if (!is_setting_enabled(Setting::DinEnabled) && _serial.allocated(io::common::Allocatable::Interface::Uart))
        {
            return sys::Config::Status::SerialPeripheralAllocatedError;
        }

        return sys::Config::Status::Ack;
    };

    switch (setting)
    {
    case Setting::RunningStatus:
    {
        // this setting applies to din midi only
        result = din_available();
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
        result = din_available();

        if (result == sys::Config::Status::Ack)
        {
            if (value)
            {
                din_midi_init_action = io::common::InitAction::Init;
            }
            else
            {
                din_midi_init_action = io::common::InitAction::DeInit;
            }

            refresh_thru = true;
        }
    }
    break;

    case Setting::BleEnabled:
    {
        if (_ble.supported())
        {
            if (value)
            {
                ble_midi_init_action = io::common::InitAction::Init;
            }
            else
            {
                ble_midi_init_action = io::common::InitAction::DeInit;
            }

            refresh_thru = true;
            result       = sys::Config::Status::Ack;
        }
        else
        {
            result = sys::Config::Status::ErrorNotSupported;
        }
    }
    break;

    case Setting::DinThruDin:
    {
        result       = din_available();
        refresh_thru = result == sys::Config::Status::Ack;
    }
    break;

    case Setting::DinThruUsb:
    {
        result = din_available();

        if ((result == sys::Config::Status::Ack) && !_usb.supported())
        {
            result = sys::Config::Status::ErrorNotSupported;
        }

        refresh_thru = result == sys::Config::Status::Ack;
    }
    break;

    case Setting::DinThruBle:
    {
        result = din_available();

        if (result == sys::Config::Status::Ack)
        {
            if (_ble.supported())
            {
                refresh_thru = true;
            }
            else
            {
                result = sys::Config::Status::ErrorNotSupported;
            }
        }
    }
    break;

    case Setting::UsbThruDin:
    {
        result = din_available();

        if ((result == sys::Config::Status::Ack) && !_usb.supported())
        {
            result = sys::Config::Status::ErrorNotSupported;
        }

        refresh_thru = result == sys::Config::Status::Ack;
    }
    break;

    case Setting::UsbThruUsb:
    {
        if (_usb.supported())
        {
            refresh_thru = true;
            result       = sys::Config::Status::Ack;
        }
        else
        {
            result = sys::Config::Status::ErrorNotSupported;
        }
    }
    break;

    case Setting::UsbThruBle:
    {
        if (_usb.supported() && _ble.supported())
        {
            refresh_thru = true;
            result       = sys::Config::Status::Ack;
        }
        else
        {
            result = sys::Config::Status::ErrorNotSupported;
        }
    }
    break;

    case Setting::BleThruDin:
    {
        result = din_available();

        if (result == sys::Config::Status::Ack)
        {
            if (_ble.supported())
            {
                refresh_thru = true;
            }
            else
            {
                result = sys::Config::Status::ErrorNotSupported;
            }
        }
    }
    break;

    case Setting::BleThruUsb:
    {
        if (_ble.supported() && _usb.supported())
        {
            refresh_thru = true;
            result       = sys::Config::Status::Ack;
        }
        else
        {
            result = sys::Config::Status::ErrorNotSupported;
        }
    }
    break;

    case Setting::BleThruBle:
    {
        if (_ble.supported())
        {
            refresh_thru = true;
            result       = sys::Config::Status::Ack;
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

        if (refresh_thru)
        {
            result = setup_thru() ? sys::Config::Status::Ack : sys::Config::Status::ErrorWrite;
        }
    }

    return result;
}
