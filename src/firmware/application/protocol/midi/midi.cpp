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
#include "application/util/logger/logger.h"
#include "application/global/midi_program.h"
#include "application/global/bpm.h"

#include "core/mcu.h"

using namespace protocol::midi;

Midi::Midi(HwaUsb&    hwaUSB,
           HwaSerial& hwaSerial,
           HwaBle&    hwaBLE,
           Database&  database)
    : _hwaUsb(hwaUSB)
    , _hwaSerial(hwaSerial)
    , _hwaBle(hwaBLE)
    , _database(database)
{
    // place all interfaces in array for easier access
    _midiInterface[INTERFACE_USB]    = &_usb;
    _midiInterface[INTERFACE_SERIAL] = &_serial;
    _midiInterface[INTERFACE_BLE]    = &_ble;

    MidiDispatcher.listen(messaging::eventType_t::ANALOG,
                          [this](const messaging::Event& event)
                          {
                              send(messaging::eventType_t::ANALOG, event);
                          });

    MidiDispatcher.listen(messaging::eventType_t::BUTTON,
                          [this](const messaging::Event& event)
                          {
                              send(messaging::eventType_t::BUTTON, event);
                          });

    MidiDispatcher.listen(messaging::eventType_t::ENCODER,
                          [this](const messaging::Event& event)
                          {
                              send(messaging::eventType_t::ENCODER, event);
                          });

    MidiDispatcher.listen(messaging::eventType_t::TOUCHSCREEN_BUTTON,
                          [this](const messaging::Event& event)
                          {
                              send(messaging::eventType_t::TOUCHSCREEN_BUTTON, event);
                          });

    MidiDispatcher.listen(messaging::eventType_t::SYSTEM,
                          [this](const messaging::Event& event)
                          {
                              switch (event.systemMessage)
                              {
                              case messaging::systemMessage_t::SYS_EX_RESPONSE:
                              {
                                  send(messaging::eventType_t::SYSTEM, event);
                              }
                              break;

                              case messaging::systemMessage_t::PRESET_CHANGED:
                              {
                                  init();
                              }
                              break;

                              case messaging::systemMessage_t::MIDI_BPM_CHANGE:
                              {
                                  if (isSettingEnabled(setting_t::DIN_ENABLED) && _clockTimerAllocated)
                                  {
                                      core::mcu::timers::setPeriod(_clockTimerIndex, Bpm.bpmToUsec(Bpm.value()));
                                      core::mcu::timers::start(_clockTimerIndex);
                                  }
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

bool Midi::init()
{
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
    if (!_serial.deInit())
    {
        return false;
    }

    if (!_usb.deInit())
    {
        return false;
    }

    if (!_ble.deInit())
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

    _usb.setNoteOffMode(isSettingEnabled(setting_t::STANDARD_NOTE_OFF) ? noteOffType_t::STANDARD_NOTE_OFF : noteOffType_t::NOTE_ON_ZERO_VEL);
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
        _serial.deInit();
    }

    _serial.setNoteOffMode(isSettingEnabled(setting_t::STANDARD_NOTE_OFF) ? noteOffType_t::STANDARD_NOTE_OFF : noteOffType_t::NOTE_ON_ZERO_VEL);
    _hwaSerial.setLoopback(isDinLoopbackRequired());

    if (!_clockTimerAllocated)
    {
        if (core::mcu::timers::allocate(_clockTimerIndex, [this]()
                                        {
                                            _serial.sendRealTime(messageType_t::SYS_REAL_TIME_CLOCK);
                                        }))
        {
            _clockTimerAllocated = true;

            core::mcu::timers::setPeriod(_clockTimerIndex, Bpm.bpmToUsec(Bpm.value()));

            if (isSettingEnabled(setting_t::SEND_MIDI_CLOCK_DIN))
            {
                core::mcu::timers::start(_clockTimerIndex);
            }
        }
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
        _ble.deInit();
    }

    _ble.setNoteOffMode(isSettingEnabled(setting_t::STANDARD_NOTE_OFF) ? noteOffType_t::STANDARD_NOTE_OFF : noteOffType_t::NOTE_ON_ZERO_VEL);

    return true;
}

bool Midi::setupThru()
{
    if (isSettingEnabled(setting_t::DIN_THRU_DIN))
    {
        _serial.registerThruInterface(_serial.transport());
    }
    else
    {
        _serial.unregisterThruInterface(_serial.transport());
    }

    if (isSettingEnabled(setting_t::DIN_THRU_USB))
    {
        _serial.registerThruInterface(_usb.transport());
    }
    else
    {
        _serial.unregisterThruInterface(_usb.transport());
    }

    if (isSettingEnabled(setting_t::DIN_THRU_BLE))
    {
        _serial.registerThruInterface(_serial.transport());
    }
    else
    {
        _serial.unregisterThruInterface(_serial.transport());
    }

    if (isSettingEnabled(setting_t::USB_THRU_DIN))
    {
        _usb.registerThruInterface(_serial.transport());
    }
    else
    {
        _usb.unregisterThruInterface(_serial.transport());
    }

    if (isSettingEnabled(setting_t::USB_THRU_USB))
    {
        _usb.registerThruInterface(_usb.transport());
    }
    else
    {
        _usb.unregisterThruInterface(_usb.transport());
    }

    if (isSettingEnabled(setting_t::USB_THRU_BLE))
    {
        _usb.registerThruInterface(_ble.transport());
    }
    else
    {
        _usb.unregisterThruInterface(_ble.transport());
    }

    if (isSettingEnabled(setting_t::BLE_THRU_DIN))
    {
        _ble.registerThruInterface(_serial.transport());
    }
    else
    {
        _ble.unregisterThruInterface(_serial.transport());
    }

    if (isSettingEnabled(setting_t::BLE_THRU_USB))
    {
        _ble.registerThruInterface(_usb.transport());
    }
    else
    {
        _ble.unregisterThruInterface(_usb.transport());
    }

    if (isSettingEnabled(setting_t::BLE_THRU_BLE))
    {
        _ble.registerThruInterface(_ble.transport());
    }
    else
    {
        _ble.unregisterThruInterface(_ble.transport());
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

        while (interfaceInstance->read())
        {
            LOG_INF("Received MIDI message on interface index %d", static_cast<int>(i));

            messaging::Event event = {};
            event.componentIndex   = 0;
            event.channel          = interfaceInstance->channel();
            event.index            = interfaceInstance->data1();
            event.value            = interfaceInstance->data2();
            event.message          = interfaceInstance->type();

            switch (event.message)
            {
            case messageType_t::SYS_EX:
            {
                // process sysex messages only from usb interface
                if (i == INTERFACE_USB)
                {
                    event.sysEx       = interfaceInstance->sysExArray();
                    event.sysExLength = interfaceInstance->length();
                }
            }
            break;

            case messageType_t::PROGRAM_CHANGE:
            {
                MidiProgram.setProgram(event.channel, event.index);
            }
            break;

            case messageType_t::NOTE_OFF:
            {
                event.value = 0;
            }
            break;

            default:
                break;
            }

            MidiDispatcher.notify(messaging::eventType_t::MIDI_IN, event);
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

void Midi::send(messaging::eventType_t source, const messaging::Event& event)
{
    using namespace protocol;

    // if omni channel is defined, send the message on each midi channel
    const uint8_t GLOBAL_CHANNEL = _database.read(database::Config::Section::global_t::MIDI_SETTINGS, setting_t::GLOBAL_CHANNEL);
    const uint8_t CHANNEL        = _database.read(database::Config::Section::global_t::MIDI_SETTINGS,
                                           setting_t::USE_GLOBAL_CHANNEL)
                                       ? GLOBAL_CHANNEL
                                       : event.channel;

    const bool USE_OMNI = CHANNEL == OMNI_CHANNEL ? true : false;

    for (size_t i = 0; i < _midiInterface.size(); i++)
    {
        auto interfaceInstance = _midiInterface[i];

        if (!interfaceInstance->initialized())
        {
            continue;
        }

        LOG_INF("MIDI interface: #%d, channel: %d, event.index: %d, event.value: %d",
                static_cast<int>(i),
                CHANNEL,
                event.index,
                event.value);

        switch (event.message)
        {
        case messageType_t::NOTE_OFF:
        {
            if (USE_OMNI)
            {
                for (uint8_t channel = 1; channel <= 16; channel++)
                {
                    interfaceInstance->sendNoteOff(event.index, event.value, channel);
                }
            }
            else
            {
                interfaceInstance->sendNoteOff(event.index, event.value, CHANNEL);
            }
        }
        break;

        case messageType_t::NOTE_ON:
        {
            if (USE_OMNI)
            {
                for (uint8_t channel = 1; channel <= 16; channel++)
                {
                    interfaceInstance->sendNoteOn(event.index, event.value, channel);
                }
            }
            else
            {
                interfaceInstance->sendNoteOn(event.index, event.value, CHANNEL);
            }
        }
        break;

        case messageType_t::CONTROL_CHANGE:
        {
            if (USE_OMNI)
            {
                for (uint8_t channel = 1; channel <= 16; channel++)
                {
                    interfaceInstance->sendControlChange(event.index, event.value, channel);
                }
            }
            else
            {
                interfaceInstance->sendControlChange(event.index, event.value, CHANNEL);
            }
        }
        break;

        case messageType_t::PROGRAM_CHANGE:
        {
            if (USE_OMNI)
            {
                for (uint8_t channel = 1; channel <= 16; channel++)
                {
                    interfaceInstance->sendProgramChange(event.index, channel);
                }
            }
            else
            {
                interfaceInstance->sendProgramChange(event.index, CHANNEL);
            }
        }
        break;

        case messageType_t::AFTER_TOUCH_CHANNEL:
        {
            if (USE_OMNI)
            {
                for (uint8_t channel = 1; channel <= 16; channel++)
                {
                    interfaceInstance->sendAfterTouch(event.value, channel);
                }
            }
            else
            {
                interfaceInstance->sendAfterTouch(event.value, CHANNEL);
            }
        }
        break;

        case messageType_t::AFTER_TOUCH_POLY:
        {
            if (USE_OMNI)
            {
                for (uint8_t channel = 1; channel <= 16; channel++)
                {
                    interfaceInstance->sendAfterTouch(event.value, channel, event.index);
                }
            }
            else
            {
                interfaceInstance->sendAfterTouch(event.value, CHANNEL, event.index);
            }
        }
        break;

        case messageType_t::PITCH_BEND:
        {
            if (USE_OMNI)
            {
                for (uint8_t channel = 1; channel <= 16; channel++)
                {
                    interfaceInstance->sendPitchBend(event.value, channel);
                }
            }
            else
            {
                interfaceInstance->sendPitchBend(event.value, CHANNEL);
            }
        }
        break;

        case messageType_t::SYS_REAL_TIME_CLOCK:
        {
            interfaceInstance->sendRealTime(event.message);
        }
        break;

        case messageType_t::SYS_REAL_TIME_START:
        {
            interfaceInstance->sendRealTime(event.message);
        }
        break;

        case messageType_t::SYS_REAL_TIME_CONTINUE:
        {
            interfaceInstance->sendRealTime(event.message);
        }
        break;

        case messageType_t::SYS_REAL_TIME_STOP:
        {
            interfaceInstance->sendRealTime(event.message);
        }
        break;

        case messageType_t::SYS_REAL_TIME_ACTIVE_SENSING:
        {
            interfaceInstance->sendRealTime(event.message);
        }
        break;

        case messageType_t::SYS_REAL_TIME_SYSTEM_RESET:
        {
            interfaceInstance->sendRealTime(event.message);
        }
        break;

        case messageType_t::MMC_PLAY:
        {
            interfaceInstance->sendMMC(event.index, event.message);
        }
        break;

        case messageType_t::MMC_STOP:
        {
            interfaceInstance->sendMMC(event.index, event.message);
        }
        break;

        case messageType_t::MMC_PAUSE:
        {
            interfaceInstance->sendMMC(event.index, event.message);
        }
        break;

        case messageType_t::MMC_RECORD_START:
        {
            interfaceInstance->sendMMC(event.index, event.message);
        }
        break;

        case messageType_t::MMC_RECORD_STOP:
        {
            interfaceInstance->sendMMC(event.index, event.message);
        }
        break;

        case messageType_t::NRPN_7BIT:
        {
            if (USE_OMNI)
            {
                for (uint8_t channel = 1; channel <= 16; channel++)
                {
                    interfaceInstance->sendNRPN(event.index, event.value, channel, false);
                }
            }
            else
            {
                interfaceInstance->sendNRPN(event.index, event.value, CHANNEL, false);
            }
        }
        break;

        case messageType_t::NRPN_14BIT:
        {
            if (USE_OMNI)
            {
                for (uint8_t channel = 1; channel <= 16; channel++)
                {
                    interfaceInstance->sendNRPN(event.index, event.value, channel, true);
                }
            }
            else
            {
                interfaceInstance->sendNRPN(event.index, event.value, CHANNEL, true);
            }
        }
        break;

        case messageType_t::CONTROL_CHANGE_14BIT:
        {
            if (USE_OMNI)
            {
                for (uint8_t channel = 1; channel <= 16; channel++)
                {
                    interfaceInstance->sendControlChange14bit(event.index, event.value, channel);
                }
            }
            else
            {
                interfaceInstance->sendControlChange14bit(event.index, event.value, CHANNEL);
            }
        }
        break;

        case messageType_t::SYS_EX:
        {
            if (source == messaging::eventType_t::SYSTEM)
            {
                if (i != INTERFACE_USB)
                {
                    // send internal sysex messages on USB interface only
                    break;
                }
            }

            interfaceInstance->sendSysEx(event.sysExLength, event.sysEx, true);
        }
        break;

        default:
            break;
        }
    }
}

// helper function used to apply note off to all available interfaces
void Midi::setNoteOffMode(noteOffType_t type)
{
    for (size_t i = 0; i < _midiInterface.size(); i++)
    {
        _midiInterface[i]->setNoteOffMode(type);
    }
}

std::optional<uint8_t> Midi::sysConfigGet(sys::Config::Section::global_t section, size_t index, uint16_t& value)
{
    if (section != sys::Config::Section::global_t::MIDI_SETTINGS)
    {
        return std::nullopt;
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
        if (!_clockTimerAllocated)
        {
            result = sys::Config::Status::ERROR_NOT_SUPPORTED;
        }
        else
        {
            result = _database.read(util::Conversion::SYS_2_DB_SECTION(section), index, readValue)
                         ? sys::Config::Status::ACK
                         : sys::Config::Status::ERROR_READ;
        }
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
        return std::nullopt;
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
                _serial.setRunningStatusState(value);
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

                result = sys::Config::Status::ACK;
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
                _serial.registerThruInterface(_serial.transport());
            }
            else
            {
                _serial.unregisterThruInterface(_serial.transport());
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
                _serial.registerThruInterface(_usb.transport());
            }
            else
            {
                _serial.unregisterThruInterface(_usb.transport());
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
                        _serial.registerThruInterface(_ble.transport());
                    }
                    else
                    {
                        _serial.unregisterThruInterface(_ble.transport());
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
                _usb.registerThruInterface(_serial.transport());
            }
            else
            {
                _usb.unregisterThruInterface(_serial.transport());
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
            _usb.registerThruInterface(_usb.transport());
        }
        else
        {
            _usb.unregisterThruInterface(_usb.transport());
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
                _usb.registerThruInterface(_ble.transport());
            }
            else
            {
                _usb.unregisterThruInterface(_ble.transport());
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
                        _ble.registerThruInterface(_serial.transport());
                    }
                    else
                    {
                        _ble.unregisterThruInterface(_serial.transport());
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
                _ble.registerThruInterface(_usb.transport());
            }
            else
            {
                _ble.unregisterThruInterface(_usb.transport());
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
                _ble.registerThruInterface(_ble.transport());
            }
            else
            {
                _ble.unregisterThruInterface(_ble.transport());
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
        if (!_clockTimerAllocated)
        {
            result = sys::Config::Status::ERROR_NOT_SUPPORTED;
        }
        else
        {
            value ? core::mcu::timers::start(_clockTimerIndex) : core::mcu::timers::stop(_clockTimerIndex);
            result = sys::Config::Status::ACK;
        }
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
        }
        break;

        case io::common::initAction_t::DE_INIT:
        {
            _serial.deInit();
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
            _ble.deInit();
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