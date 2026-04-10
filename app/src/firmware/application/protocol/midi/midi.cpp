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

#include "midi.h"
#include "application/system/config.h"
#include "application/util/conversion/conversion.h"
#include "application/messaging/messaging.h"
#include "application/util/configurable/configurable.h"
#include "application/global/midi_program.h"
#include "application/global/bpm.h"

#include <zephyr/logging/log.h>

using namespace protocol::midi;
using namespace zlibs::utils;

namespace
{
    LOG_MODULE_REGISTER(database, CONFIG_OPENDECK_LOG_LEVEL);    // NOLINT

    constexpr uint8_t MIDI_GROUP = 0;

    uint8_t to_zero_based_channel(uint8_t channel)
    {
        return channel > 0 ? channel - 1 : 0;
    }

}    // namespace

Midi::Midi(HwaUsb&    hwaUSB,
           HwaSerial& hwaSerial,
           HwaBle&    hwaBLE,
           Database&  database)
    : _hwaUsb(hwaUSB)
    , _hwaSerial(hwaSerial)
    , _hwaBle(hwaBLE)
    , _database(database)
    , _clockTimer(
          misc::Timer::Type::Repeating,
          [this]()
          {
              _serial.send(lib::midi::midi1::timing_clock(MIDI_GROUP));
          })
{
    // place all interfaces in array for easier access
    _midiInterface[INTERFACE_USB]    = &_usb;
    _midiInterface[INTERFACE_SERIAL] = &_serial;
    _midiInterface[INTERFACE_BLE]    = &_ble;

    messaging::subscribe<messaging::SystemSignal>(
        [this](const messaging::SystemSignal& event)
        {
            switch (event.systemMessage)
            {
            case messaging::systemMessage_t::FACTORY_RESET_START:
            {
                _usb.deinit();
            }
            break;

            case messaging::systemMessage_t::PRESET_CHANGED:
            {
                init();
            }
            break;

            case messaging::systemMessage_t::MIDI_BPM_CHANGE:
            {
                if (isSettingEnabled(setting_t::DIN_ENABLED) && isSettingEnabled(setting_t::SEND_MIDI_CLOCK_DIN))
                {
                    _clockTimer.start(Bpm.bpmToUsec(Bpm.value()), misc::TimerUnit::Us);
                }
            }
            break;

            default:
                break;
            }
        });

    messaging::subscribe<messaging::MidiSignal>(
        [this](const messaging::MidiSignal& event)
        {
            send(event);
        });

    messaging::subscribe<messaging::UmpSignal>(
        [this](const messaging::UmpSignal& event)
        {
            if (event.direction != messaging::MidiDirection::Out)
            {
                return;
            }

            send(event);
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

bool Midi::init()
{
    _standardNoteOff = isSettingEnabled(setting_t::STANDARD_NOTE_OFF);

    if (!setupUsb())
    {
        return false;
    }

    if (!setupSerial())
    {
        return false;
    }

    if (!setupBle())
    {
        return false;
    }

    if (!setupThru())
    {
        return false;
    }

    return true;
}

bool Midi::deInit()
{
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

bool Midi::setupUsb()
{
    if (!_usb.init())
    {
        return false;
    }

    return true;
}

bool Midi::setupSerial()
{
    if (isSettingEnabled(setting_t::DIN_ENABLED))
    {
        _serial.init();
    }
    else
    {
        _serial.deinit();
    }

    _hwaSerial.setLoopback(isDinLoopbackRequired());

    if (isSettingEnabled(setting_t::SEND_MIDI_CLOCK_DIN))
    {
        _clockTimer.start(Bpm.bpmToUsec(Bpm.value()), misc::TimerUnit::Us);
    }
    else
    {
        _clockTimer.stop();
    }

    return true;
}

bool Midi::setupBle()
{
    if (isSettingEnabled(setting_t::BLE_ENABLED))
    {
        _ble.init();
    }
    else
    {
        _ble.deinit();
    }

    return true;
}

bool Midi::setupThru()
{
    if (isSettingEnabled(setting_t::DIN_THRU_DIN))
    {
        _serial.register_thru_interface(_serial.thru_interface());
    }
    else
    {
        _serial.unregister_thru_interface(_serial.thru_interface());
    }

    if (isSettingEnabled(setting_t::DIN_THRU_USB))
    {
        _serial.register_thru_interface(_usb.thru_interface());
    }
    else
    {
        _serial.unregister_thru_interface(_usb.thru_interface());
    }

    if (isSettingEnabled(setting_t::DIN_THRU_BLE))
    {
        _serial.register_thru_interface(_ble.thru_interface());
    }
    else
    {
        _serial.unregister_thru_interface(_ble.thru_interface());
    }

    if (isSettingEnabled(setting_t::USB_THRU_DIN))
    {
        _usb.register_thru_interface(_serial.thru_interface());
    }
    else
    {
        _usb.unregister_thru_interface(_serial.thru_interface());
    }

    if (isSettingEnabled(setting_t::USB_THRU_USB))
    {
        _usb.register_thru_interface(_usb.thru_interface());
    }
    else
    {
        _usb.unregister_thru_interface(_usb.thru_interface());
    }

    if (isSettingEnabled(setting_t::USB_THRU_BLE))
    {
        _usb.register_thru_interface(_ble.thru_interface());
    }
    else
    {
        _usb.unregister_thru_interface(_ble.thru_interface());
    }

    if (isSettingEnabled(setting_t::BLE_THRU_DIN))
    {
        _ble.register_thru_interface(_serial.thru_interface());
    }
    else
    {
        _ble.unregister_thru_interface(_serial.thru_interface());
    }

    if (isSettingEnabled(setting_t::BLE_THRU_USB))
    {
        _ble.register_thru_interface(_usb.thru_interface());
    }
    else
    {
        _ble.unregister_thru_interface(_usb.thru_interface());
    }

    if (isSettingEnabled(setting_t::BLE_THRU_BLE))
    {
        _ble.register_thru_interface(_ble.thru_interface());
    }
    else
    {
        _ble.unregister_thru_interface(_ble.thru_interface());
    }

    return true;
}

void Midi::read()
{
    for (size_t i = 0; i < _midiInterface.size(); i++)
    {
        auto interfaceInstance = _midiInterface[i];

        if (!interfaceInstance->initialized())
        {
            continue;
        }

        while (true)
        {
            const auto PACKET = interfaceInstance->read();

            if (!PACKET.has_value())
            {
                break;
            }

            messaging::publish(messaging::MidiTrafficSignal{
                .transport = interfaceToTransport(i),
                .direction = messaging::MidiDirection::In,
            });

            const auto message = midi::decode_message(PACKET.value());

            switch (message.type)
            {
            case messageType_t::SYS_EX:
            {
                // process sysex messages only from usb interface
                if (i != INTERFACE_USB)
                {
                    continue;
                }
            }
            break;

            case messageType_t::PROGRAM_CHANGE:
            {
                MidiProgram.setProgram(message.channel, message.data1);
            }
            break;

            default:
                break;
            }

            messaging::publish(messaging::UmpSignal{
                .direction = messaging::MidiDirection::In,
                .packet    = PACKET.value(),
            });
        }
    }
}

bool Midi::isSettingEnabled(setting_t feature)
{
    return _database.read(database::Config::Section::global_t::MIDI_SETTINGS, feature);
}

bool Midi::isDinLoopbackRequired()
{
    return (isSettingEnabled(setting_t::DIN_ENABLED) &&
            isSettingEnabled(setting_t::DIN_THRU_DIN) &&
            !isSettingEnabled(setting_t::DIN_THRU_USB) &&
            !isSettingEnabled(setting_t::DIN_THRU_BLE));
}

void Midi::send(const messaging::MidiSignal& event)
{
    if (event.source == messaging::MidiSource::AnalogButton)
    {
        return;
    }

    const uint8_t GLOBAL_CHANNEL = _database.read(database::Config::Section::global_t::MIDI_SETTINGS, setting_t::GLOBAL_CHANNEL);
    const uint8_t CHANNEL        = _database.read(database::Config::Section::global_t::MIDI_SETTINGS,
                                                  setting_t::USE_GLOBAL_CHANNEL)
                                       ? GLOBAL_CHANNEL
                                       : event.channel;
    const bool    USE_OMNI       = CHANNEL == OMNI_CHANNEL;

    auto send_ump = [this](const midi_ump& packet)
    {
        messaging::UmpSignal signal = {};
        signal.packet               = packet;
        send(signal);
    };

    auto send_channel_voice = [&](auto builder)
    {
        if (USE_OMNI)
        {
            for (uint8_t channel = 1; channel <= 16; channel++)
            {
                send_ump(builder(channel));
            }
        }
        else
        {
            send_ump(builder(CHANNEL));
        }
    };

    switch (event.message)
    {
    case messageType_t::NOTE_OFF:
    {
        send_channel_voice(
            [&](uint8_t channel)
            {
                return lib::midi::midi1::note_off(
                    MIDI_GROUP,
                    to_zero_based_channel(channel),
                    event.index,
                    event.value,
                    !_standardNoteOff);
            });
    }
    break;

    case messageType_t::NOTE_ON:
    {
        send_channel_voice(
            [&](uint8_t channel)
            {
                return lib::midi::midi1::note_on(
                    MIDI_GROUP,
                    to_zero_based_channel(channel),
                    event.index,
                    event.value);
            });
    }
    break;

    case messageType_t::CONTROL_CHANGE:
    {
        send_channel_voice(
            [&](uint8_t channel)
            {
                return lib::midi::midi1::control_change(
                    MIDI_GROUP,
                    to_zero_based_channel(channel),
                    event.index,
                    event.value);
            });
    }
    break;

    case messageType_t::PROGRAM_CHANGE:
    {
        send_channel_voice(
            [&](uint8_t channel)
            {
                return lib::midi::midi1::program_change(
                    MIDI_GROUP,
                    to_zero_based_channel(channel),
                    event.index);
            });
    }
    break;

    case messageType_t::SYS_REAL_TIME_CLOCK:
        send_ump(lib::midi::midi1::timing_clock(MIDI_GROUP));
        break;

    case messageType_t::SYS_REAL_TIME_START:
        send_ump(lib::midi::midi1::start(MIDI_GROUP));
        break;

    case messageType_t::SYS_REAL_TIME_CONTINUE:
        send_ump(lib::midi::midi1::continue_playback(MIDI_GROUP));
        break;

    case messageType_t::SYS_REAL_TIME_STOP:
        send_ump(lib::midi::midi1::stop(MIDI_GROUP));
        break;

    case messageType_t::SYS_REAL_TIME_ACTIVE_SENSING:
        send_ump(lib::midi::midi1::active_sensing(MIDI_GROUP));
        break;

    case messageType_t::SYS_REAL_TIME_SYSTEM_RESET:
        send_ump(lib::midi::midi1::system_reset(MIDI_GROUP));
        break;

    case messageType_t::MMC_PLAY:
    case messageType_t::MMC_STOP:
    case messageType_t::MMC_PAUSE:
    case messageType_t::MMC_RECORD_START:
    case messageType_t::MMC_RECORD_STOP:
    {
        std::optional<midi_ump> packet = std::nullopt;

        switch (event.message)
        {
        case messageType_t::MMC_PLAY:
            packet = lib::midi::midi1::mmc<lib::midi::MessageType::MmcPlay>(MIDI_GROUP, event.index);
            break;

        case messageType_t::MMC_STOP:
            packet = lib::midi::midi1::mmc<lib::midi::MessageType::MmcStop>(MIDI_GROUP, event.index);
            break;

        case messageType_t::MMC_PAUSE:
            packet = lib::midi::midi1::mmc<lib::midi::MessageType::MmcPause>(MIDI_GROUP, event.index);
            break;

        case messageType_t::MMC_RECORD_START:
            packet = lib::midi::midi1::mmc<lib::midi::MessageType::MmcRecordStart>(MIDI_GROUP, event.index);
            break;

        case messageType_t::MMC_RECORD_STOP:
            packet = lib::midi::midi1::mmc<lib::midi::MessageType::MmcRecordStop>(MIDI_GROUP, event.index);
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

void Midi::send(const messaging::UmpSignal& event)
{
    for (size_t i = 0; i < _midiInterface.size(); i++)
    {
        auto* interfaceInstance = _midiInterface[i];

        if ((interfaceInstance != nullptr) && interfaceInstance->initialized())
        {
            interfaceInstance->send(event.packet);

            messaging::publish(messaging::MidiTrafficSignal{
                .transport = interfaceToTransport(i),
                .direction = messaging::MidiDirection::Out,
            });
        }
    }
}

messaging::MidiTransport Midi::interfaceToTransport(size_t index) const
{
    switch (index)
    {
    case INTERFACE_USB:
        return messaging::MidiTransport::Usb;

    case INTERFACE_SERIAL:
        return messaging::MidiTransport::Din;

    case INTERFACE_BLE:
        return messaging::MidiTransport::Ble;

    default:
        return messaging::MidiTransport::Usb;
    }
}

void Midi::setNoteOffMode(noteOffType_t type)
{
    _standardNoteOff = type == noteOffType_t::STANDARD_NOTE_OFF;
}

std::optional<uint8_t> Midi::sysConfigGet(sys::Config::Section::global_t section, size_t index, uint16_t& value)
{
    if (section != sys::Config::Section::global_t::MIDI_SETTINGS)
    {
        return {};
    }

    [[maybe_unused]] uint32_t readValue = 0;
    [[maybe_unused]] uint8_t  result    = sys::Config::Status::ERROR_READ;

    auto feature = static_cast<setting_t>(index);

    switch (feature)
    {
    case setting_t::STANDARD_NOTE_OFF:
    {
        result = _database.read(util::Conversion::SYS_2_DB_SECTION(section), index, readValue)
                     ? sys::Config::Status::ACK
                     : sys::Config::Status::ERROR_READ;
    }
    break;

    case setting_t::RUNNING_STATUS:
    case setting_t::DIN_ENABLED:
    case setting_t::DIN_THRU_DIN:
    case setting_t::DIN_THRU_USB:
    case setting_t::USB_THRU_DIN:
    {
        if (_hwaSerial.supported())
        {
            if (!isSettingEnabled(setting_t::DIN_ENABLED) && _hwaSerial.allocated(io::common::Allocatable::interface_t::UART))
            {
                result = sys::Config::Status::SERIAL_PERIPHERAL_ALLOCATED_ERROR;
            }
            else
            {
                result = _database.read(util::Conversion::SYS_2_DB_SECTION(section), index, readValue)
                             ? sys::Config::Status::ACK
                             : sys::Config::Status::ERROR_READ;
            }
        }
        else
        {
            result = sys::Config::Status::ERROR_NOT_SUPPORTED;
        }
    }
    break;

    case setting_t::BLE_ENABLED:
    case setting_t::BLE_THRU_USB:
    case setting_t::BLE_THRU_BLE:
    case setting_t::USB_THRU_BLE:
    {
        if (_hwaBle.supported())
        {
            result = _database.read(util::Conversion::SYS_2_DB_SECTION(section), index, readValue)
                         ? sys::Config::Status::ACK
                         : sys::Config::Status::ERROR_READ;
        }
        else
        {
            result = sys::Config::Status::ERROR_NOT_SUPPORTED;
        }
    }
    break;

    case setting_t::DIN_THRU_BLE:
    case setting_t::BLE_THRU_DIN:
    {
        if (_hwaSerial.supported())
        {
            if (!isSettingEnabled(setting_t::DIN_ENABLED) && _hwaSerial.allocated(io::common::Allocatable::interface_t::UART))
            {
                result = sys::Config::Status::SERIAL_PERIPHERAL_ALLOCATED_ERROR;
            }
            else
            {
                if (_hwaBle.supported())
                {
                    result = _database.read(util::Conversion::SYS_2_DB_SECTION(section), index, readValue)
                                 ? sys::Config::Status::ACK
                                 : sys::Config::Status::ERROR_READ;
                }
                else
                {
                    result = sys::Config::Status::ERROR_NOT_SUPPORTED;
                }
            }
        }
        else
        {
            result = sys::Config::Status::ERROR_NOT_SUPPORTED;
        }
    }
    break;

    case setting_t::SEND_MIDI_CLOCK_DIN:
    {
        result = _database.read(util::Conversion::SYS_2_DB_SECTION(section), index, readValue)
                     ? sys::Config::Status::ACK
                     : sys::Config::Status::ERROR_READ;
    }
    break;

    default:
    {
        result = _database.read(util::Conversion::SYS_2_DB_SECTION(section), index, readValue)
                     ? sys::Config::Status::ACK
                     : sys::Config::Status::ERROR_READ;
    }
    break;
    }

    value = readValue;
    return result;
}

std::optional<uint8_t> Midi::sysConfigSet(sys::Config::Section::global_t section, size_t index, uint16_t value)
{
    if (section != sys::Config::Section::global_t::MIDI_SETTINGS)
    {
        return {};
    }

    [[maybe_unused]] uint8_t result            = sys::Config::Status::ERROR_WRITE;
    [[maybe_unused]] bool    writeToDb         = true;
    [[maybe_unused]] auto    dinMIDIinitAction = io::common::initAction_t::AS_IS;
    [[maybe_unused]] auto    bleMIDIinitAction = io::common::initAction_t::AS_IS;
    [[maybe_unused]] bool    checkDINLoopback  = false;

    auto setting = static_cast<setting_t>(index);

    switch (setting)
    {
    case setting_t::RUNNING_STATUS:
    {
        // this setting applies to din midi only
        if (_hwaSerial.supported())
        {
            if (!isSettingEnabled(setting_t::DIN_ENABLED) && _hwaSerial.allocated(io::common::Allocatable::interface_t::UART))
            {
                result = sys::Config::Status::SERIAL_PERIPHERAL_ALLOCATED_ERROR;
            }
            else
            {
                // Running status handling is internal to the zlibs serial parser.
                result = sys::Config::Status::ACK;
            }
        }
        else
        {
            result = sys::Config::Status::ERROR_NOT_SUPPORTED;
        }
    }
    break;

    case setting_t::STANDARD_NOTE_OFF:
    {
        setNoteOffMode(value ? noteOffType_t::STANDARD_NOTE_OFF : noteOffType_t::NOTE_ON_ZERO_VEL);
        result = sys::Config::Status::ACK;
    }
    break;

    case setting_t::DIN_ENABLED:
    {
        if (_hwaSerial.supported())
        {
            if (!isSettingEnabled(setting_t::DIN_ENABLED) && _hwaSerial.allocated(io::common::Allocatable::interface_t::UART))
            {
                result = sys::Config::Status::SERIAL_PERIPHERAL_ALLOCATED_ERROR;
            }
            else
            {
                if (value)
                {
                    dinMIDIinitAction = io::common::initAction_t::INIT;
                }
                else
                {
                    dinMIDIinitAction = io::common::initAction_t::DE_INIT;
                }

                checkDINLoopback = true;
                result           = sys::Config::Status::ACK;
            }
        }
        else
        {
            result = sys::Config::Status::ERROR_NOT_SUPPORTED;
        }
    }
    break;

    case setting_t::BLE_ENABLED:
    {
        if (_hwaBle.supported())
        {
            if (value)
            {
                bleMIDIinitAction = io::common::initAction_t::INIT;
            }
            else
            {
                bleMIDIinitAction = io::common::initAction_t::DE_INIT;
            }

            result = sys::Config::Status::ACK;
        }
        else
        {
            result = sys::Config::Status::ERROR_NOT_SUPPORTED;
        }
    }
    break;

    case setting_t::DIN_THRU_DIN:
    {
        if (_hwaSerial.supported())
        {
            if (value)
            {
                _serial.register_thru_interface(_serial.thru_interface());
            }
            else
            {
                _serial.unregister_thru_interface(_serial.thru_interface());
            }

            result           = sys::Config::Status::ACK;
            checkDINLoopback = true;
        }
        else
        {
            result = sys::Config::Status::ERROR_NOT_SUPPORTED;
        }
    }
    break;

    case setting_t::DIN_THRU_USB:
    {
        if (_hwaSerial.supported())
        {
            if (value)
            {
                _serial.register_thru_interface(_usb.thru_interface());
            }
            else
            {
                _serial.unregister_thru_interface(_usb.thru_interface());
            }

            result           = sys::Config::Status::ACK;
            checkDINLoopback = true;
        }
        else
        {
            result = sys::Config::Status::ERROR_NOT_SUPPORTED;
        }
    }
    break;

    case setting_t::DIN_THRU_BLE:
    {
        if (_hwaSerial.supported())
        {
            if (!isSettingEnabled(setting_t::DIN_ENABLED) && _hwaSerial.allocated(io::common::Allocatable::interface_t::UART))
            {
                result = sys::Config::Status::SERIAL_PERIPHERAL_ALLOCATED_ERROR;
            }
            else
            {
                if (_hwaBle.supported())
                {
                    if (value)
                    {
                        _serial.register_thru_interface(_ble.thru_interface());
                    }
                    else
                    {
                        _serial.unregister_thru_interface(_ble.thru_interface());
                    }

                    result           = sys::Config::Status::ACK;
                    checkDINLoopback = true;
                }
                else
                {
                    result = sys::Config::Status::ERROR_NOT_SUPPORTED;
                }
            }
        }
        else
        {
            result = sys::Config::Status::ERROR_NOT_SUPPORTED;
        }
    }
    break;

    case setting_t::USB_THRU_DIN:
    {
        if (_hwaSerial.supported())
        {
            if (value)
            {
                _usb.register_thru_interface(_serial.thru_interface());
            }
            else
            {
                _usb.unregister_thru_interface(_serial.thru_interface());
            }

            result = sys::Config::Status::ACK;
        }
        else
        {
            result = sys::Config::Status::ERROR_NOT_SUPPORTED;
        }
    }
    break;

    case setting_t::USB_THRU_USB:
    {
        if (value)
        {
            _usb.register_thru_interface(_usb.thru_interface());
        }
        else
        {
            _usb.unregister_thru_interface(_usb.thru_interface());
        }

        result = sys::Config::Status::ACK;
    }
    break;

    case setting_t::USB_THRU_BLE:
    {
        if (_hwaBle.supported())
        {
            if (value)
            {
                _usb.register_thru_interface(_ble.thru_interface());
            }
            else
            {
                _usb.unregister_thru_interface(_ble.thru_interface());
            }

            result = sys::Config::Status::ACK;
        }
        else
        {
            result = sys::Config::Status::ERROR_NOT_SUPPORTED;
        }
    }
    break;

    case setting_t::BLE_THRU_DIN:
    {
        if (_hwaSerial.supported())
        {
            if (!isSettingEnabled(setting_t::DIN_ENABLED) && _hwaSerial.allocated(io::common::Allocatable::interface_t::UART))
            {
                result = sys::Config::Status::SERIAL_PERIPHERAL_ALLOCATED_ERROR;
            }
            else
            {
                if (_hwaBle.supported())
                {
                    if (value)
                    {
                        _ble.register_thru_interface(_serial.thru_interface());
                    }
                    else
                    {
                        _ble.unregister_thru_interface(_serial.thru_interface());
                    }

                    result = sys::Config::Status::ACK;
                }
                else
                {
                    result = sys::Config::Status::ERROR_NOT_SUPPORTED;
                }
            }
        }
        else
        {
            result = sys::Config::Status::ERROR_NOT_SUPPORTED;
        }
    }
    break;

    case setting_t::BLE_THRU_USB:
    {
        if (_hwaBle.supported())
        {
            if (value)
            {
                _ble.register_thru_interface(_usb.thru_interface());
            }
            else
            {
                _ble.unregister_thru_interface(_usb.thru_interface());
            }

            result = sys::Config::Status::ACK;
        }
        else
        {
            result = sys::Config::Status::ERROR_NOT_SUPPORTED;
        }
    }
    break;

    case setting_t::BLE_THRU_BLE:
    {
        if (_hwaBle.supported())
        {
            if (value)
            {
                _ble.register_thru_interface(_ble.thru_interface());
            }
            else
            {
                _ble.unregister_thru_interface(_ble.thru_interface());
            }

            result = sys::Config::Status::ACK;
        }
        else
        {
            result = sys::Config::Status::ERROR_NOT_SUPPORTED;
        }
    }
    break;

    case setting_t::GLOBAL_CHANNEL:
    {
        if ((value < 1) || (value > OMNI_CHANNEL))
        {
            // invalid channel
            result = sys::Config::Status::ERROR_NEW_VALUE;
            break;
        }

        result = sys::Config::Status::ACK;
    }
    break;

    case setting_t::SEND_MIDI_CLOCK_DIN:
    {
        if (value)
        {
            _clockTimer.start(Bpm.bpmToUsec(Bpm.value()), misc::TimerUnit::Us);
        }
        else
        {
            _clockTimer.stop();
        }
        result = sys::Config::Status::ACK;
    }
    break;

    default:
    {
        result = sys::Config::Status::ACK;
    }
    break;
    }

    if ((result == sys::Config::Status::ACK) && writeToDb)
    {
        result = _database.update(util::Conversion::SYS_2_DB_SECTION(section), index, value)
                     ? sys::Config::Status::ACK
                     : sys::Config::Status::ERROR_WRITE;

        switch (dinMIDIinitAction)
        {
        case io::common::initAction_t::INIT:
        {
            _serial.init();

            if (isSettingEnabled(setting_t::SEND_MIDI_CLOCK_DIN))
            {
                _clockTimer.start(Bpm.bpmToUsec(Bpm.value()), misc::TimerUnit::Us);
            }
        }
        break;

        case io::common::initAction_t::DE_INIT:
        {
            if (isSettingEnabled(setting_t::SEND_MIDI_CLOCK_DIN))
            {
                _clockTimer.stop();
            }

            _serial.deinit();
        }
        break;

        default:
            break;
        }

        switch (bleMIDIinitAction)
        {
        case io::common::initAction_t::INIT:
        {
            _ble.init();
        }
        break;

        case io::common::initAction_t::DE_INIT:
        {
            _ble.deinit();
        }
        break;

        default:
            break;
        }
    }

    // no need to check this if init/deinit has been already called for DIN
    if (result && checkDINLoopback && dinMIDIinitAction == io::common::initAction_t::AS_IS)
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
        _hwaSerial.setLoopback(isDinLoopbackRequired());
    }

    return result;
}
