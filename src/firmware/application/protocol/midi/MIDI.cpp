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
#include "application/system/Config.h"
#include "application/util/conversion/Conversion.h"
#include "application/messaging/Messaging.h"
#include "application/util/configurable/Configurable.h"
#include "application/util/logger/Logger.h"
#include "application/global/MIDIProgram.h"

using namespace io;

protocol::MIDI::MIDI(HWAUSB&   hwaUSB,
                     HWADIN&   hwaDIN,
                     HWABLE&   hwaBLE,
                     Database& database)
    : _hwaUSB(hwaUSB)
    , _hwaDIN(hwaDIN)
    , _hwaBLE(hwaBLE)
    , _database(database)
{
    // place all interfaces in array for easier access
    _midiInterface[INTERFACE_USB] = &_usbMIDI;
    _midiInterface[INTERFACE_DIN] = &_dinMIDI;
    _midiInterface[INTERFACE_BLE] = &_bleMIDI;

    MIDIDispatcher.listen(messaging::eventType_t::ANALOG,
                          [this](const messaging::event_t& event)
                          {
                              sendMIDI(messaging::eventType_t::ANALOG, event);
                          });

    MIDIDispatcher.listen(messaging::eventType_t::BUTTON,
                          [this](const messaging::event_t& event)
                          {
                              sendMIDI(messaging::eventType_t::BUTTON, event);
                          });

    MIDIDispatcher.listen(messaging::eventType_t::ENCODER,
                          [this](const messaging::event_t& event)
                          {
                              sendMIDI(messaging::eventType_t::ENCODER, event);
                          });

    MIDIDispatcher.listen(messaging::eventType_t::TOUCHSCREEN_BUTTON,
                          [this](const messaging::event_t& event)
                          {
                              sendMIDI(messaging::eventType_t::TOUCHSCREEN_BUTTON, event);
                          });

    MIDIDispatcher.listen(messaging::eventType_t::SYSTEM,
                          [this](const messaging::event_t& event)
                          {
                              switch (event.systemMessage)
                              {
                              case messaging::systemMessage_t::SYS_EX_RESPONSE:
                              {
                                  sendMIDI(messaging::eventType_t::SYSTEM, event);
                              }
                              break;

                              case messaging::systemMessage_t::PRESET_CHANGED:
                              {
                                  init();
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

bool protocol::MIDI::init()
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

bool protocol::MIDI::deInit()
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

bool protocol::MIDI::setupUSBMIDI()
{
    if (!_usbMIDI.init())
    {
        return false;
    }

    _usbMIDI.setNoteOffMode(isSettingEnabled(setting_t::STANDARD_NOTE_OFF) ? noteOffType_t::STANDARD_NOTE_OFF : noteOffType_t::NOTE_ON_ZERO_VEL);
    return true;
}

bool protocol::MIDI::setupDINMIDI()
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

bool protocol::MIDI::setupBLEMIDI()
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

bool protocol::MIDI::setupThru()
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

void protocol::MIDI::read()
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

            messaging::event_t event;

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

            MIDIDispatcher.notify(messaging::eventType_t::MIDI_IN, event);
        }
    }
}

bool protocol::MIDI::isSettingEnabled(setting_t feature)
{
    return _database.read(database::Config::Section::global_t::MIDI_SETTINGS, feature);
}

bool protocol::MIDI::isDinLoopbackRequired()
{
    return (isSettingEnabled(setting_t::DIN_ENABLED) &&
            isSettingEnabled(setting_t::DIN_THRU_DIN) &&
            !isSettingEnabled(setting_t::DIN_THRU_USB) &&
            !isSettingEnabled(setting_t::DIN_THRU_BLE));
}

void protocol::MIDI::sendMIDI(messaging::eventType_t source, const messaging::event_t& event)
{
    using namespace protocol;

    // if omni channel is defined, send the message on each midi channel
    const uint8_t GLOBAL_CHANNEL = _database.read(database::Config::Section::global_t::MIDI_SETTINGS, setting_t::GLOBAL_CHANNEL);
    const uint8_t CHANNEL        = _database.read(database::Config::Section::global_t::MIDI_SETTINGS,
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
void protocol::MIDI::setNoteOffMode(noteOffType_t type)
{
    for (size_t i = 0; i < _midiInterface.size(); i++)
    {
        _midiInterface[i]->setNoteOffMode(type);
    }
}

std::optional<uint8_t> protocol::MIDI::sysConfigGet(sys::Config::Section::global_t section, size_t index, uint16_t& value)
{
    [[maybe_unused]] uint32_t readValue = 0;
    [[maybe_unused]] uint8_t  result    = sys::Config::status_t::ERROR_READ;

    switch (section)
    {
    case sys::Config::Section::global_t::MIDI_SETTINGS:
    {
        auto feature = static_cast<setting_t>(index);

        switch (feature)
        {
        case setting_t::STANDARD_NOTE_OFF:
        {
            result = _database.read(util::Conversion::SYS_2_DB_SECTION(section), index, readValue)
                         ? sys::Config::status_t::ACK
                         : sys::Config::status_t::ERROR_READ;
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
                if (!isSettingEnabled(setting_t::DIN_ENABLED) && _hwaDIN.allocated(io::common::Allocatable::interface_t::UART))
                {
                    result = sys::Config::status_t::SERIAL_PERIPHERAL_ALLOCATED_ERROR;
                }
                else
                {
                    result = _database.read(util::Conversion::SYS_2_DB_SECTION(section), index, readValue)
                                 ? sys::Config::status_t::ACK
                                 : sys::Config::status_t::ERROR_READ;
                }
            }
            else
            {
                result = sys::Config::status_t::ERROR_NOT_SUPPORTED;
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
                result = _database.read(util::Conversion::SYS_2_DB_SECTION(section), index, readValue)
                             ? sys::Config::status_t::ACK
                             : sys::Config::status_t::ERROR_READ;
            }
            else
            {
                result = sys::Config::status_t::ERROR_NOT_SUPPORTED;
            }
        }
        break;

        case setting_t::DIN_THRU_BLE:
        case setting_t::BLE_THRU_DIN:
        {
            if (_hwaDIN.supported())
            {
                if (!isSettingEnabled(setting_t::DIN_ENABLED) && _hwaDIN.allocated(io::common::Allocatable::interface_t::UART))
                {
                    result = sys::Config::status_t::SERIAL_PERIPHERAL_ALLOCATED_ERROR;
                }
                else
                {
                    if (_hwaBLE.supported())
                    {
                        result = _database.read(util::Conversion::SYS_2_DB_SECTION(section), index, readValue)
                                     ? sys::Config::status_t::ACK
                                     : sys::Config::status_t::ERROR_READ;
                    }
                    else
                    {
                        result = sys::Config::status_t::ERROR_NOT_SUPPORTED;
                    }
                }
            }
            else
            {
                result = sys::Config::status_t::ERROR_NOT_SUPPORTED;
            }
        }
        break;

        default:
        {
            result = _database.read(util::Conversion::SYS_2_DB_SECTION(section), index, readValue)
                         ? sys::Config::status_t::ACK
                         : sys::Config::status_t::ERROR_READ;
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

std::optional<uint8_t> protocol::MIDI::sysConfigSet(sys::Config::Section::global_t section, size_t index, uint16_t value)
{
    [[maybe_unused]] uint8_t result            = sys::Config::status_t::ERROR_WRITE;
    [[maybe_unused]] bool    writeToDb         = true;
    [[maybe_unused]] auto    dinMIDIinitAction = common::initAction_t::AS_IS;
    [[maybe_unused]] auto    bleMIDIinitAction = common::initAction_t::AS_IS;
    [[maybe_unused]] bool    checkDINLoopback  = false;

    switch (section)
    {
    case sys::Config::Section::global_t::MIDI_SETTINGS:
    {
        auto setting = static_cast<setting_t>(index);

        switch (setting)
        {
        case setting_t::RUNNING_STATUS:
        {
            // this setting applies to din midi only
            if (_hwaDIN.supported())
            {
                if (!isSettingEnabled(setting_t::DIN_ENABLED) && _hwaDIN.allocated(io::common::Allocatable::interface_t::UART))
                {
                    result = sys::Config::status_t::SERIAL_PERIPHERAL_ALLOCATED_ERROR;
                }
                else
                {
                    _dinMIDI.setRunningStatusState(value);
                    result = sys::Config::status_t::ACK;
                }
            }
            else
            {
                result = sys::Config::status_t::ERROR_NOT_SUPPORTED;
            }
        }
        break;

        case setting_t::STANDARD_NOTE_OFF:
        {
            setNoteOffMode(value ? noteOffType_t::STANDARD_NOTE_OFF : noteOffType_t::NOTE_ON_ZERO_VEL);
            result = sys::Config::status_t::ACK;
        }
        break;

        case setting_t::DIN_ENABLED:
        {
            if (_hwaDIN.supported())
            {
                if (!isSettingEnabled(setting_t::DIN_ENABLED) && _hwaDIN.allocated(io::common::Allocatable::interface_t::UART))
                {
                    result = sys::Config::status_t::SERIAL_PERIPHERAL_ALLOCATED_ERROR;
                }
                else
                {
                    if (value)
                    {
                        dinMIDIinitAction = common::initAction_t::INIT;
                    }
                    else
                    {
                        dinMIDIinitAction = common::initAction_t::DE_INIT;
                    }

                    result = sys::Config::status_t::ACK;
                }
            }
            else
            {
                result = sys::Config::status_t::ERROR_NOT_SUPPORTED;
            }
        }
        break;

        case setting_t::BLE_ENABLED:
        {
            if (_hwaBLE.supported())
            {
                if (value)
                {
                    bleMIDIinitAction = common::initAction_t::INIT;
                }
                else
                {
                    bleMIDIinitAction = common::initAction_t::DE_INIT;
                }

                result = sys::Config::status_t::ACK;
            }
            else
            {
                result = sys::Config::status_t::ERROR_NOT_SUPPORTED;
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

                result           = sys::Config::status_t::ACK;
                checkDINLoopback = true;
            }
            else
            {
                result = sys::Config::status_t::ERROR_NOT_SUPPORTED;
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

                result           = sys::Config::status_t::ACK;
                checkDINLoopback = true;
            }
            else
            {
                result = sys::Config::status_t::ERROR_NOT_SUPPORTED;
            }
        }
        break;

        case setting_t::DIN_THRU_BLE:
        {
            if (_hwaDIN.supported())
            {
                if (!isSettingEnabled(setting_t::DIN_ENABLED) && _hwaDIN.allocated(io::common::Allocatable::interface_t::UART))
                {
                    result = sys::Config::status_t::SERIAL_PERIPHERAL_ALLOCATED_ERROR;
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

                        result           = sys::Config::status_t::ACK;
                        checkDINLoopback = true;
                    }
                    else
                    {
                        result = sys::Config::status_t::ERROR_NOT_SUPPORTED;
                    }
                }
            }
            else
            {
                result = sys::Config::status_t::ERROR_NOT_SUPPORTED;
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

                result = sys::Config::status_t::ACK;
            }
            else
            {
                result = sys::Config::status_t::ERROR_NOT_SUPPORTED;
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

            result = sys::Config::status_t::ACK;
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

                result = sys::Config::status_t::ACK;
            }
            else
            {
                result = sys::Config::status_t::ERROR_NOT_SUPPORTED;
            }
        }
        break;

        case setting_t::BLE_THRU_DIN:
        {
            if (_hwaDIN.supported())
            {
                if (!isSettingEnabled(setting_t::DIN_ENABLED) && _hwaDIN.allocated(io::common::Allocatable::interface_t::UART))
                {
                    result = sys::Config::status_t::SERIAL_PERIPHERAL_ALLOCATED_ERROR;
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

                        result = sys::Config::status_t::ACK;
                    }
                    else
                    {
                        result = sys::Config::status_t::ERROR_NOT_SUPPORTED;
                    }
                }
            }
            else
            {
                result = sys::Config::status_t::ERROR_NOT_SUPPORTED;
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

                result = sys::Config::status_t::ACK;
            }
            else
            {
                result = sys::Config::status_t::ERROR_NOT_SUPPORTED;
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

                result = sys::Config::status_t::ACK;
            }
            else
            {
                result = sys::Config::status_t::ERROR_NOT_SUPPORTED;
            }
        }
        break;

        case setting_t::GLOBAL_CHANNEL:
        {
            if ((value < 1) || (value > MIDI_CHANNEL_OMNI))
            {
                // invalid channel
                result = sys::Config::status_t::ERROR_NEW_VALUE;
                break;
            }

            result = sys::Config::status_t::ACK;
        }
        break;

        default:
        {
            result = sys::Config::status_t::ACK;
        }
        break;
        }
    }
    break;

    default:
        return std::nullopt;
    }

    if ((result == sys::Config::status_t::ACK) && writeToDb)
    {
        result = _database.update(util::Conversion::SYS_2_DB_SECTION(section), index, value)
                     ? sys::Config::status_t::ACK
                     : sys::Config::status_t::ERROR_WRITE;

        switch (dinMIDIinitAction)
        {
        case common::initAction_t::INIT:
        {
            _dinMIDI.init();
        }
        break;

        case common::initAction_t::DE_INIT:
        {
            _dinMIDI.deInit();
        }
        break;

        default:
            break;
        }

        switch (bleMIDIinitAction)
        {
        case common::initAction_t::INIT:
        {
            _bleMIDI.init();
        }
        break;

        case common::initAction_t::DE_INIT:
        {
            _bleMIDI.deInit();
        }
        break;

        default:
            break;
        }
    }

    // no need to check this if init/deinit has been already called for DIN
    if (result && checkDINLoopback && dinMIDIinitAction == common::initAction_t::AS_IS)
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