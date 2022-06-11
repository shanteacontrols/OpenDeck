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

#include "MIDI.h"
#include "system/Config.h"
#include "util/conversion/Conversion.h"
#include "messaging/Messaging.h"
#include "util/configurable/Configurable.h"
#include "logger/Logger.h"
#include "global/MIDIProgram.h"

using namespace IO;

Protocol::MIDI::MIDI(HWAUSB&             hwaUSB,
                     HWADIN&             hwaDIN,
                     HWABLE&             hwaBLE,
                     Database::Instance& database)
    : _hwaUSB(hwaUSB)
    , _hwaDIN(hwaDIN)
    , _hwaBLE(hwaBLE)
    , _database(database)
{
    // place all interfaces in array for easier access
    _midiInterface[INTERFACE_USB] = &_usbMIDI;
    _midiInterface[INTERFACE_DIN] = &_dinMIDI;
    _midiInterface[INTERFACE_BLE] = &_bleMIDI;

    MIDIDispatcher.listen(Messaging::eventType_t::ANALOG,
                          [this](const Messaging::event_t& event)
                          {
                              sendMIDI(Messaging::eventType_t::ANALOG, event);
                          });

    MIDIDispatcher.listen(Messaging::eventType_t::BUTTON,
                          [this](const Messaging::event_t& event)
                          {
                              sendMIDI(Messaging::eventType_t::BUTTON, event);
                          });

    MIDIDispatcher.listen(Messaging::eventType_t::ENCODER,
                          [this](const Messaging::event_t& event)
                          {
                              sendMIDI(Messaging::eventType_t::ENCODER, event);
                          });

    MIDIDispatcher.listen(Messaging::eventType_t::TOUCHSCREEN_BUTTON,
                          [this](const Messaging::event_t& event)
                          {
                              sendMIDI(Messaging::eventType_t::TOUCHSCREEN_BUTTON, event);
                          });

    MIDIDispatcher.listen(Messaging::eventType_t::SYSTEM,
                          [this](const Messaging::event_t& event)
                          {
                              switch (event.systemMessage)
                              {
                              case Messaging::systemMessage_t::SYS_EX_RESPONSE:
                              {
                                  sendMIDI(Messaging::eventType_t::SYSTEM, event);
                              }
                              break;

                              default:
                                  break;
                              }
                          });

    MIDIDispatcher.listen(Messaging::eventType_t::PRESET,
                          [this](const Messaging::event_t& event)
                          {
                              init();
                          });

    ConfigHandler.registerConfig(
        System::Config::block_t::GLOBAL,
        // read
        [this](uint8_t section, size_t index, uint16_t& value)
        {
            return sysConfigGet(static_cast<System::Config::Section::global_t>(section), index, value);
        },

        // write
        [this](uint8_t section, size_t index, uint16_t value)
        {
            return sysConfigSet(static_cast<System::Config::Section::global_t>(section), index, value);
        });
}

bool Protocol::MIDI::init()
{
    if (!setupUSBMIDI())
    {
        return false;
    }

    if (!setupDINMIDI())
    {
        return false;
    }

    if (!setupBLEMIDI())
    {
        return false;
    }

    if (!setupThru())
    {
        return false;
    }

    return true;
}

bool Protocol::MIDI::deInit()
{
    if (!_dinMIDI.deInit())
    {
        return false;
    }

    if (!_usbMIDI.deInit())
    {
        return false;
    }

    if (!_bleMIDI.deInit())
    {
        return false;
    }

    return true;
}

bool Protocol::MIDI::setupUSBMIDI()
{
    if (!_usbMIDI.init())
    {
        return false;
    }

    _usbMIDI.setNoteOffMode(isSettingEnabled(setting_t::STANDARD_NOTE_OFF) ? noteOffType_t::STANDARD_NOTE_OFF : noteOffType_t::NOTE_ON_ZERO_VEL);
    return true;
}

bool Protocol::MIDI::setupDINMIDI()
{
    if (isSettingEnabled(setting_t::DIN_ENABLED))
    {
        _dinMIDI.init();
    }
    else
    {
        _dinMIDI.deInit();
    }

    _dinMIDI.setNoteOffMode(isSettingEnabled(setting_t::STANDARD_NOTE_OFF) ? noteOffType_t::STANDARD_NOTE_OFF : noteOffType_t::NOTE_ON_ZERO_VEL);
    _hwaDIN.setLoopback(isDinLoopbackRequired());

    return true;
}

bool Protocol::MIDI::setupBLEMIDI()
{
    if (isSettingEnabled(setting_t::BLE_ENABLED))
    {
        _bleMIDI.init();
    }
    else
    {
        _bleMIDI.deInit();
    }

    _bleMIDI.setNoteOffMode(isSettingEnabled(setting_t::STANDARD_NOTE_OFF) ? noteOffType_t::STANDARD_NOTE_OFF : noteOffType_t::NOTE_ON_ZERO_VEL);

    return true;
}

bool Protocol::MIDI::setupThru()
{
    if (isSettingEnabled(setting_t::DIN_THRU_DIN))
    {
        _dinMIDI.registerThruInterface(_dinMIDI.transport());
    }
    else
    {
        _dinMIDI.unregisterThruInterface(_dinMIDI.transport());
    }

    if (isSettingEnabled(setting_t::DIN_THRU_USB))
    {
        _dinMIDI.registerThruInterface(_usbMIDI.transport());
    }
    else
    {
        _dinMIDI.unregisterThruInterface(_usbMIDI.transport());
    }

    if (isSettingEnabled(setting_t::DIN_THRU_BLE))
    {
        _dinMIDI.registerThruInterface(_dinMIDI.transport());
    }
    else
    {
        _dinMIDI.unregisterThruInterface(_dinMIDI.transport());
    }

    if (isSettingEnabled(setting_t::USB_THRU_DIN))
    {
        _usbMIDI.registerThruInterface(_dinMIDI.transport());
    }
    else
    {
        _usbMIDI.unregisterThruInterface(_dinMIDI.transport());
    }

    if (isSettingEnabled(setting_t::USB_THRU_USB))
    {
        _usbMIDI.registerThruInterface(_usbMIDI.transport());
    }
    else
    {
        _usbMIDI.unregisterThruInterface(_usbMIDI.transport());
    }

    if (isSettingEnabled(setting_t::USB_THRU_BLE))
    {
        _usbMIDI.registerThruInterface(_bleMIDI.transport());
    }
    else
    {
        _usbMIDI.unregisterThruInterface(_bleMIDI.transport());
    }

    if (isSettingEnabled(setting_t::BLE_THRU_DIN))
    {
        _bleMIDI.registerThruInterface(_dinMIDI.transport());
    }
    else
    {
        _bleMIDI.unregisterThruInterface(_dinMIDI.transport());
    }

    if (isSettingEnabled(setting_t::BLE_THRU_USB))
    {
        _bleMIDI.registerThruInterface(_usbMIDI.transport());
    }
    else
    {
        _bleMIDI.unregisterThruInterface(_usbMIDI.transport());
    }

    if (isSettingEnabled(setting_t::BLE_THRU_BLE))
    {
        _bleMIDI.registerThruInterface(_bleMIDI.transport());
    }
    else
    {
        _bleMIDI.unregisterThruInterface(_bleMIDI.transport());
    }

    return true;
}

void Protocol::MIDI::read()
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
            LOG_INFO("Received MIDI message on interface index %d", static_cast<int>(i));

            Messaging::event_t event;

            event.componentIndex = 0;
            event.channel        = interfaceInstance->channel();
            event.index          = interfaceInstance->data1();
            event.value          = interfaceInstance->data2();
            event.message        = interfaceInstance->type();

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
                MIDIProgram.setProgram(event.channel, event.index);
                _database.setPreset(event.index);
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

            MIDIDispatcher.notify(Messaging::eventType_t::MIDI_IN, event);
        }
    }
}

bool Protocol::MIDI::isSettingEnabled(setting_t feature)
{
    return _database.read(Database::Config::Section::global_t::MIDI_SETTINGS, feature);
}

bool Protocol::MIDI::isDinLoopbackRequired()
{
    return (isSettingEnabled(setting_t::DIN_ENABLED) &&
            isSettingEnabled(setting_t::DIN_THRU_DIN) &&
            !isSettingEnabled(setting_t::DIN_THRU_USB) &&
            !isSettingEnabled(setting_t::DIN_THRU_BLE));
}

void Protocol::MIDI::sendMIDI(Messaging::eventType_t source, const Messaging::event_t& event)
{
    using namespace Protocol;

    // if omni channel is defined, send the message on each midi channel
    const uint8_t GLOBAL_CHANNEL = _database.read(Database::Config::Section::global_t::MIDI_SETTINGS, setting_t::GLOBAL_CHANNEL);
    const uint8_t CHANNEL        = _database.read(Database::Config::Section::global_t::MIDI_SETTINGS,
                                           setting_t::USE_GLOBAL_CHANNEL)
                                       ? GLOBAL_CHANNEL
                                       : event.channel;

    const bool USE_OMNI = CHANNEL == MIDI_CHANNEL_OMNI ? true : false;

    for (size_t i = 0; i < _midiInterface.size(); i++)
    {
        auto interfaceInstance = _midiInterface[i];

        if (!interfaceInstance->initialized())
        {
            continue;
        }

        LOG_INFO("Sending MIDI message in interface index %d", static_cast<int>(i));

        switch (event.message)
        {
        case MIDI::messageType_t::NOTE_OFF:
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

        case MIDI::messageType_t::NOTE_ON:
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

        case MIDI::messageType_t::CONTROL_CHANGE:
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

        case MIDI::messageType_t::PROGRAM_CHANGE:
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

        case MIDI::messageType_t::AFTER_TOUCH_CHANNEL:
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

        case MIDI::messageType_t::AFTER_TOUCH_POLY:
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

        case MIDI::messageType_t::PITCH_BEND:
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

        case MIDI::messageType_t::SYS_REAL_TIME_CLOCK:
        {
            interfaceInstance->sendRealTime(event.message);
        }
        break;

        case MIDI::messageType_t::SYS_REAL_TIME_START:
        {
            interfaceInstance->sendRealTime(event.message);
        }
        break;

        case MIDI::messageType_t::SYS_REAL_TIME_CONTINUE:
        {
            interfaceInstance->sendRealTime(event.message);
        }
        break;

        case MIDI::messageType_t::SYS_REAL_TIME_STOP:
        {
            interfaceInstance->sendRealTime(event.message);
        }
        break;

        case MIDI::messageType_t::SYS_REAL_TIME_ACTIVE_SENSING:
        {
            interfaceInstance->sendRealTime(event.message);
        }
        break;

        case MIDI::messageType_t::SYS_REAL_TIME_SYSTEM_RESET:
        {
            interfaceInstance->sendRealTime(event.message);
        }
        break;

        case MIDI::messageType_t::MMC_PLAY:
        {
            interfaceInstance->sendMMC(event.index, event.message);
        }
        break;

        case MIDI::messageType_t::MMC_STOP:
        {
            interfaceInstance->sendMMC(event.index, event.message);
        }
        break;

        case MIDI::messageType_t::MMC_PAUSE:
        {
            interfaceInstance->sendMMC(event.index, event.message);
        }
        break;

        case MIDI::messageType_t::MMC_RECORD_START:
        {
            interfaceInstance->sendMMC(event.index, event.message);
        }
        break;

        case MIDI::messageType_t::MMC_RECORD_STOP:
        {
            interfaceInstance->sendMMC(event.index, event.message);
        }
        break;

        case MIDI::messageType_t::NRPN_7BIT:
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

        case MIDI::messageType_t::NRPN_14BIT:
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

        case MIDI::messageType_t::CONTROL_CHANGE_14BIT:
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

        case MIDI::messageType_t::SYS_EX:
        {
            if (source == Messaging::eventType_t::SYSTEM)
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
void Protocol::MIDI::setNoteOffMode(noteOffType_t type)
{
    for (size_t i = 0; i < _midiInterface.size(); i++)
    {
        _midiInterface[i]->setNoteOffMode(type);
    }
}

std::optional<uint8_t> Protocol::MIDI::sysConfigGet(System::Config::Section::global_t section, size_t index, uint16_t& value)
{
    [[maybe_unused]] int32_t readValue = 0;
    [[maybe_unused]] uint8_t result    = System::Config::status_t::ERROR_READ;

    switch (section)
    {
    case System::Config::Section::global_t::MIDI_SETTINGS:
    {
        auto feature = static_cast<setting_t>(index);

        switch (feature)
        {
        case setting_t::STANDARD_NOTE_OFF:
        {
            result = _database.read(Util::Conversion::sys2DBsection(section), index, readValue) ? System::Config::status_t::ACK : System::Config::status_t::ERROR_READ;
        }
        break;

        case setting_t::RUNNING_STATUS:
        case setting_t::DIN_ENABLED:
        case setting_t::DIN_THRU_DIN:
        case setting_t::DIN_THRU_USB:
        case setting_t::USB_THRU_DIN:
        {
            if (_hwaDIN.supported())
            {
                if (!isSettingEnabled(setting_t::DIN_ENABLED) && _hwaDIN.allocated(IO::Common::Allocatable::interface_t::UART))
                {
                    result = System::Config::status_t::SERIAL_PERIPHERAL_ALLOCATED_ERROR;
                }
                else
                {
                    result = _database.read(Util::Conversion::sys2DBsection(section), index, readValue) ? System::Config::status_t::ACK : System::Config::status_t::ERROR_READ;
                }
            }
            else
            {
                result = System::Config::status_t::ERROR_NOT_SUPPORTED;
            }
        }
        break;

        case setting_t::BLE_ENABLED:
        case setting_t::BLE_THRU_USB:
        case setting_t::BLE_THRU_BLE:
        case setting_t::USB_THRU_BLE:
        {
            if (_hwaBLE.supported())
            {
                result = _database.read(Util::Conversion::sys2DBsection(section), index, readValue) ? System::Config::status_t::ACK : System::Config::status_t::ERROR_READ;
            }
            else
            {
                result = System::Config::status_t::ERROR_NOT_SUPPORTED;
            }
        }
        break;

        case setting_t::DIN_THRU_BLE:
        case setting_t::BLE_THRU_DIN:
        {
            if (_hwaDIN.supported())
            {
                if (!isSettingEnabled(setting_t::DIN_ENABLED) && _hwaDIN.allocated(IO::Common::Allocatable::interface_t::UART))
                {
                    result = System::Config::status_t::SERIAL_PERIPHERAL_ALLOCATED_ERROR;
                }
                else
                {
                    if (_hwaBLE.supported())
                    {
                        result = _database.read(Util::Conversion::sys2DBsection(section), index, readValue) ? System::Config::status_t::ACK : System::Config::status_t::ERROR_READ;
                    }
                    else
                    {
                        result = System::Config::status_t::ERROR_NOT_SUPPORTED;
                    }
                }
            }
            else
            {
                result = System::Config::status_t::ERROR_NOT_SUPPORTED;
            }
        }
        break;

        default:
        {
            result = _database.read(Util::Conversion::sys2DBsection(section), index, readValue) ? System::Config::status_t::ACK : System::Config::status_t::ERROR_READ;
        }
        break;
        }
    }
    break;

    default:
        return std::nullopt;
    }

    value = readValue;
    return result;
}

std::optional<uint8_t> Protocol::MIDI::sysConfigSet(System::Config::Section::global_t section, size_t index, uint16_t value)
{
    [[maybe_unused]] uint8_t result            = System::Config::status_t::ERROR_WRITE;
    [[maybe_unused]] bool    writeToDb         = true;
    [[maybe_unused]] auto    dinMIDIinitAction = Common::initAction_t::AS_IS;
    [[maybe_unused]] auto    bleMIDIinitAction = Common::initAction_t::AS_IS;
    [[maybe_unused]] bool    checkDINLoopback  = false;

    switch (section)
    {
    case System::Config::Section::global_t::MIDI_SETTINGS:
    {
        auto setting = static_cast<setting_t>(index);

        switch (setting)
        {
        case setting_t::RUNNING_STATUS:
        {
            // this setting applies to din midi only
            if (_hwaDIN.supported())
            {
                if (!isSettingEnabled(setting_t::DIN_ENABLED) && _hwaDIN.allocated(IO::Common::Allocatable::interface_t::UART))
                {
                    result = System::Config::status_t::SERIAL_PERIPHERAL_ALLOCATED_ERROR;
                }
                else
                {
                    _dinMIDI.setRunningStatusState(value);
                    result = System::Config::status_t::ACK;
                }
            }
            else
            {
                result = System::Config::status_t::ERROR_NOT_SUPPORTED;
            }
        }
        break;

        case setting_t::STANDARD_NOTE_OFF:
        {
            setNoteOffMode(value ? noteOffType_t::STANDARD_NOTE_OFF : noteOffType_t::NOTE_ON_ZERO_VEL);
            result = System::Config::status_t::ACK;
        }
        break;

        case setting_t::DIN_ENABLED:
        {
            if (_hwaDIN.supported())
            {
                if (!isSettingEnabled(setting_t::DIN_ENABLED) && _hwaDIN.allocated(IO::Common::Allocatable::interface_t::UART))
                {
                    result = System::Config::status_t::SERIAL_PERIPHERAL_ALLOCATED_ERROR;
                }
                else
                {
                    if (value)
                    {
                        dinMIDIinitAction = Common::initAction_t::INIT;
                    }
                    else
                    {
                        dinMIDIinitAction = Common::initAction_t::DE_INIT;
                    }

                    result = System::Config::status_t::ACK;
                }
            }
            else
            {
                result = System::Config::status_t::ERROR_NOT_SUPPORTED;
            }
        }
        break;

        case setting_t::BLE_ENABLED:
        {
            if (_hwaBLE.supported())
            {
                if (value)
                {
                    bleMIDIinitAction = Common::initAction_t::INIT;
                }
                else
                {
                    bleMIDIinitAction = Common::initAction_t::DE_INIT;
                }

                result = System::Config::status_t::ACK;
            }
            else
            {
                result = System::Config::status_t::ERROR_NOT_SUPPORTED;
            }
        }
        break;

        case setting_t::DIN_THRU_DIN:
        {
            if (_hwaDIN.supported())
            {
                if (value)
                {
                    _dinMIDI.registerThruInterface(_dinMIDI.transport());
                }
                else
                {
                    _dinMIDI.unregisterThruInterface(_dinMIDI.transport());
                }

                result           = System::Config::status_t::ACK;
                checkDINLoopback = true;
            }
            else
            {
                result = System::Config::status_t::ERROR_NOT_SUPPORTED;
            }
        }
        break;

        case setting_t::DIN_THRU_USB:
        {
            if (_hwaDIN.supported())
            {
                if (value)
                {
                    _dinMIDI.registerThruInterface(_usbMIDI.transport());
                }
                else
                {
                    _dinMIDI.unregisterThruInterface(_usbMIDI.transport());
                }

                result           = System::Config::status_t::ACK;
                checkDINLoopback = true;
            }
            else
            {
                result = System::Config::status_t::ERROR_NOT_SUPPORTED;
            }
        }
        break;

        case setting_t::DIN_THRU_BLE:
        {
            if (_hwaDIN.supported())
            {
                if (!isSettingEnabled(setting_t::DIN_ENABLED) && _hwaDIN.allocated(IO::Common::Allocatable::interface_t::UART))
                {
                    result = System::Config::status_t::SERIAL_PERIPHERAL_ALLOCATED_ERROR;
                }
                else
                {
                    if (_hwaBLE.supported())
                    {
                        if (value)
                        {
                            _dinMIDI.registerThruInterface(_bleMIDI.transport());
                        }
                        else
                        {
                            _dinMIDI.unregisterThruInterface(_bleMIDI.transport());
                        }

                        result           = System::Config::status_t::ACK;
                        checkDINLoopback = true;
                    }
                    else
                    {
                        result = System::Config::status_t::ERROR_NOT_SUPPORTED;
                    }
                }
            }
            else
            {
                result = System::Config::status_t::ERROR_NOT_SUPPORTED;
            }
        }
        break;

        case setting_t::USB_THRU_DIN:
        {
            if (_hwaDIN.supported())
            {
                if (value)
                {
                    _usbMIDI.registerThruInterface(_dinMIDI.transport());
                }
                else
                {
                    _usbMIDI.unregisterThruInterface(_dinMIDI.transport());
                }

                result = System::Config::status_t::ACK;
            }
            else
            {
                result = System::Config::status_t::ERROR_NOT_SUPPORTED;
            }
        }
        break;

        case setting_t::USB_THRU_USB:
        {
            if (value)
            {
                _usbMIDI.registerThruInterface(_usbMIDI.transport());
            }
            else
            {
                _usbMIDI.unregisterThruInterface(_usbMIDI.transport());
            }

            result = System::Config::status_t::ACK;
        }
        break;

        case setting_t::USB_THRU_BLE:
        {
            if (_hwaBLE.supported())
            {
                if (value)
                {
                    _usbMIDI.registerThruInterface(_bleMIDI.transport());
                }
                else
                {
                    _usbMIDI.unregisterThruInterface(_bleMIDI.transport());
                }

                result = System::Config::status_t::ACK;
            }
            else
            {
                result = System::Config::status_t::ERROR_NOT_SUPPORTED;
            }
        }
        break;

        case setting_t::BLE_THRU_DIN:
        {
            if (_hwaDIN.supported())
            {
                if (!isSettingEnabled(setting_t::DIN_ENABLED) && _hwaDIN.allocated(IO::Common::Allocatable::interface_t::UART))
                {
                    result = System::Config::status_t::SERIAL_PERIPHERAL_ALLOCATED_ERROR;
                }
                else
                {
                    if (_hwaBLE.supported())
                    {
                        if (value)
                        {
                            _bleMIDI.registerThruInterface(_dinMIDI.transport());
                        }
                        else
                        {
                            _bleMIDI.unregisterThruInterface(_dinMIDI.transport());
                        }

                        result = System::Config::status_t::ACK;
                    }
                    else
                    {
                        result = System::Config::status_t::ERROR_NOT_SUPPORTED;
                    }
                }
            }
            else
            {
                result = System::Config::status_t::ERROR_NOT_SUPPORTED;
            }
        }
        break;

        case setting_t::BLE_THRU_USB:
        {
            if (_hwaBLE.supported())
            {
                if (value)
                {
                    _bleMIDI.registerThruInterface(_usbMIDI.transport());
                }
                else
                {
                    _bleMIDI.unregisterThruInterface(_usbMIDI.transport());
                }

                result = System::Config::status_t::ACK;
            }
            else
            {
                result = System::Config::status_t::ERROR_NOT_SUPPORTED;
            }
        }
        break;

        case setting_t::BLE_THRU_BLE:
        {
            if (_hwaBLE.supported())
            {
                if (value)
                {
                    _bleMIDI.registerThruInterface(_bleMIDI.transport());
                }
                else
                {
                    _bleMIDI.unregisterThruInterface(_bleMIDI.transport());
                }

                result = System::Config::status_t::ACK;
            }
            else
            {
                result = System::Config::status_t::ERROR_NOT_SUPPORTED;
            }
        }
        break;

        case setting_t::GLOBAL_CHANNEL:
        {
            if ((value < 1) || (value > MIDI_CHANNEL_OMNI))
            {
                // invalid channel
                result = System::Config::status_t::ERROR_NEW_VALUE;
                break;
            }

            result = System::Config::status_t::ACK;
        }
        break;

        default:
        {
            result = System::Config::status_t::ACK;
        }
        break;
        }
    }
    break;

    default:
        return std::nullopt;
    }

    if ((result == System::Config::status_t::ACK) && writeToDb)
    {
        result = _database.update(Util::Conversion::sys2DBsection(section), index, value) ? System::Config::status_t::ACK : System::Config::status_t::ERROR_WRITE;

        switch (dinMIDIinitAction)
        {
        case Common::initAction_t::INIT:
        {
            _dinMIDI.init();
        }
        break;

        case Common::initAction_t::DE_INIT:
        {
            _dinMIDI.deInit();
        }
        break;

        default:
            break;
        }

        switch (bleMIDIinitAction)
        {
        case Common::initAction_t::INIT:
        {
            _bleMIDI.init();
        }
        break;

        case Common::initAction_t::DE_INIT:
        {
            _bleMIDI.deInit();
        }
        break;

        default:
            break;
        }
    }

    // no need to check this if init/deinit has been already called for DIN
    if (result && checkDINLoopback && dinMIDIinitAction == Common::initAction_t::AS_IS)
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
        _hwaDIN.setLoopback(isDinLoopbackRequired());
    }

    return result;
}