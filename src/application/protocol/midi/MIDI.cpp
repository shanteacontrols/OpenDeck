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
#include "io/common/Common.h"
#include "system/Config.h"
#include "util/conversion/Conversion.h"
#include "messaging/Messaging.h"
#include "util/configurable/Configurable.h"
#include "logger/Logger.h"

using namespace IO;

Protocol::MIDI::MIDI(HWAUSB&             hwaUSB,
                     HWADIN&             hwaDIN,
                     Database::Instance& database)
    : _hwaUSB(hwaUSB)
    , _hwaDIN(hwaDIN)
    , _database(database)
{
    // place all interfaces in array for easier access
    _midiInterface[INTERFACE_USB] = &_usbMIDI;
    _midiInterface[INTERFACE_DIN] = &_dinMIDI;

    MIDIDispatcher.listen(Messaging::eventType_t::analog,
                          [this](const Messaging::event_t& event) {
                              sendMIDI(Messaging::eventType_t::analog, event);
                          });

    MIDIDispatcher.listen(Messaging::eventType_t::button,
                          [this](const Messaging::event_t& event) {
                              sendMIDI(Messaging::eventType_t::button, event);
                          });

    MIDIDispatcher.listen(Messaging::eventType_t::encoder,
                          [this](const Messaging::event_t& event) {
                              sendMIDI(Messaging::eventType_t::encoder, event);
                          });

    MIDIDispatcher.listen(Messaging::eventType_t::touchscreenButton,
                          [this](const Messaging::event_t& event) {
                              sendMIDI(Messaging::eventType_t::touchscreenButton, event);
                          });

    MIDIDispatcher.listen(Messaging::eventType_t::system,
                          [this](const Messaging::event_t& event) {
                              switch (event.componentIndex)
                              {
                              case static_cast<uint16_t>(Messaging::systemMessage_t::sysExResponse):
                              {
                                  sendMIDI(Messaging::eventType_t::system, event);
                              }
                              break;

                              default:
                                  break;
                              }
                          });

    MIDIDispatcher.listen(Messaging::eventType_t::preset,
                          [this](const Messaging::event_t& event) {
                              init();
                          });

    ConfigHandler.registerConfig(
        System::Config::block_t::global,
        // read
        [this](uint8_t section, size_t index, uint16_t& value) {
            return sysConfigGet(static_cast<System::Config::Section::global_t>(section), index, value);
        },

        // write
        [this](uint8_t section, size_t index, uint16_t value) {
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

    return true;
}

bool Protocol::MIDI::setupUSBMIDI()
{
    if (!_usbMIDI.init())
    {
        return false;
    }

    _usbMIDI.setNoteOffMode(isSettingEnabled(setting_t::standardNoteOff) ? noteOffType_t::standardNoteOff : noteOffType_t::noteOnZeroVel);
    return true;
}

bool Protocol::MIDI::setupDINMIDI()
{
    if (isSettingEnabled(setting_t::dinEnabled))
    {
        _dinMIDI.init();
    }
    else
    {
        _dinMIDI.deInit();
    }

    _dinMIDI.setNoteOffMode(isSettingEnabled(setting_t::standardNoteOff) ? noteOffType_t::standardNoteOff : noteOffType_t::noteOnZeroVel);
    _hwaDIN.setLoopback(isDinLoopbackRequired());

    return true;
}

bool Protocol::MIDI::setupThru()
{
    if (isSettingEnabled(setting_t::dinThruDin))
    {
        _dinMIDI.registerThruInterface(_dinMIDI.transport());
    }
    else
    {
        _dinMIDI.unregisterThruInterface(_dinMIDI.transport());
    }

    if (isSettingEnabled(setting_t::dinThruUsb))
    {
        _dinMIDI.registerThruInterface(_usbMIDI.transport());
    }
    else
    {
        _dinMIDI.unregisterThruInterface(_usbMIDI.transport());
    }

    if (isSettingEnabled(setting_t::usbThruDin))
    {
        _usbMIDI.registerThruInterface(_dinMIDI.transport());
    }
    else
    {
        _usbMIDI.unregisterThruInterface(_dinMIDI.transport());
    }

    if (isSettingEnabled(setting_t::usbThruUsb))
    {
        _usbMIDI.registerThruInterface(_usbMIDI.transport());
    }
    else
    {
        _usbMIDI.unregisterThruInterface(_usbMIDI.transport());
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
            event.midiChannel    = interfaceInstance->channel();
            event.midiIndex      = interfaceInstance->data1();
            event.midiValue      = interfaceInstance->data2();
            event.message        = interfaceInstance->type();

            switch (event.message)
            {
            case messageType_t::systemExclusive:
            {
                // process sysex messages only from usb interface
                if (i == INTERFACE_USB)
                {
                    event.sysEx       = interfaceInstance->sysExArray();
                    event.sysExLength = interfaceInstance->length();
                }
            }
            break;

            case messageType_t::programChange:
            {
                Common::setProgram(event.midiChannel, event.midiIndex);
                _database.setPreset(event.midiIndex);
            }
            break;

            case messageType_t::noteOff:
            {
                event.midiValue = 0;
            }
            break;

            default:
                break;
            }

            MIDIDispatcher.notify(Messaging::eventType_t::midiIn, event);
        }
    }
}

bool Protocol::MIDI::isSettingEnabled(setting_t feature)
{
    return _database.read(Database::Config::Section::global_t::midiSettings, feature);
}

bool Protocol::MIDI::isDinLoopbackRequired()
{
    return (isSettingEnabled(setting_t::dinEnabled) &&
            isSettingEnabled(setting_t::dinThruDin) &&
            !isSettingEnabled(setting_t::dinThruUsb) &&
            !isSettingEnabled(setting_t::dinThruBle));
}

void Protocol::MIDI::sendMIDI(Messaging::eventType_t source, const Messaging::event_t& event)
{
    using namespace Protocol;

    // if omni channel is defined, send the message on each midi channel
    const uint8_t globalChannel = _database.read(Database::Config::Section::global_t::midiSettings, setting_t::globalChannel);
    const uint8_t channel       = _database.read(Database::Config::Section::global_t::midiSettings,
                                           setting_t::useGlobalChannel)
                                      ? globalChannel
                                      : event.midiChannel;

    const bool useOmni = channel == MIDI_CHANNEL_OMNI ? true : false;

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
        case MIDI::messageType_t::noteOff:
        {
            if (useOmni)
            {
                for (uint8_t channel = 1; channel <= 16; channel++)
                {
                    interfaceInstance->sendNoteOff(event.midiIndex, event.midiValue, channel);
                }
            }
            else
            {
                interfaceInstance->sendNoteOff(event.midiIndex, event.midiValue, channel);
            }
        }
        break;

        case MIDI::messageType_t::noteOn:
        {
            if (useOmni)
            {
                for (uint8_t channel = 1; channel <= 16; channel++)
                {
                    interfaceInstance->sendNoteOn(event.midiIndex, event.midiValue, channel);
                }
            }
            else
            {
                interfaceInstance->sendNoteOn(event.midiIndex, event.midiValue, channel);
            }
        }
        break;

        case MIDI::messageType_t::controlChange:
        {
            if (useOmni)
            {
                for (uint8_t channel = 1; channel <= 16; channel++)
                {
                    interfaceInstance->sendControlChange(event.midiIndex, event.midiValue, channel);
                }
            }
            else
            {
                interfaceInstance->sendControlChange(event.midiIndex, event.midiValue, channel);
            }
        }
        break;

        case MIDI::messageType_t::programChange:
        {
            if (useOmni)
            {
                for (uint8_t channel = 1; channel <= 16; channel++)
                {
                    interfaceInstance->sendProgramChange(event.midiIndex, channel);
                }
            }
            else
            {
                interfaceInstance->sendProgramChange(event.midiIndex, channel);
            }
        }
        break;

        case MIDI::messageType_t::afterTouchChannel:
        {
            if (useOmni)
            {
                for (uint8_t channel = 1; channel <= 16; channel++)
                {
                    interfaceInstance->sendAfterTouch(event.midiValue, channel);
                }
            }
            else
            {
                interfaceInstance->sendAfterTouch(event.midiValue, channel);
            }
        }
        break;

        case MIDI::messageType_t::afterTouchPoly:
        {
            if (useOmni)
            {
                for (uint8_t channel = 1; channel <= 16; channel++)
                {
                    interfaceInstance->sendAfterTouch(event.midiValue, channel, event.midiIndex);
                }
            }
            else
            {
                interfaceInstance->sendAfterTouch(event.midiValue, channel, event.midiIndex);
            }
        }
        break;

        case MIDI::messageType_t::pitchBend:
        {
            if (useOmni)
            {
                for (uint8_t channel = 1; channel <= 16; channel++)
                {
                    interfaceInstance->sendPitchBend(event.midiValue, channel);
                }
            }
            else
            {
                interfaceInstance->sendPitchBend(event.midiValue, channel);
            }
        }
        break;

        case MIDI::messageType_t::sysRealTimeClock:
        {
            interfaceInstance->sendRealTime(event.message);
        }
        break;

        case MIDI::messageType_t::sysRealTimeStart:
        {
            interfaceInstance->sendRealTime(event.message);
        }
        break;

        case MIDI::messageType_t::sysRealTimeContinue:
        {
            interfaceInstance->sendRealTime(event.message);
        }
        break;

        case MIDI::messageType_t::sysRealTimeStop:
        {
            interfaceInstance->sendRealTime(event.message);
        }
        break;

        case MIDI::messageType_t::sysRealTimeActiveSensing:
        {
            interfaceInstance->sendRealTime(event.message);
        }
        break;

        case MIDI::messageType_t::sysRealTimeSystemReset:
        {
            interfaceInstance->sendRealTime(event.message);
        }
        break;

        case MIDI::messageType_t::mmcPlay:
        {
            interfaceInstance->sendMMC(event.midiIndex, event.message);
        }
        break;

        case MIDI::messageType_t::mmcStop:
        {
            interfaceInstance->sendMMC(event.midiIndex, event.message);
        }
        break;

        case MIDI::messageType_t::mmcPause:
        {
            interfaceInstance->sendMMC(event.midiIndex, event.message);
        }
        break;

        case MIDI::messageType_t::mmcRecordStart:
        {
            interfaceInstance->sendMMC(event.midiIndex, event.message);
        }
        break;

        case MIDI::messageType_t::mmcRecordStop:
        {
            interfaceInstance->sendMMC(event.midiIndex, event.message);
        }
        break;

        case MIDI::messageType_t::nrpn7bit:
        {
            if (useOmni)
            {
                for (uint8_t channel = 1; channel <= 16; channel++)
                {
                    interfaceInstance->sendNRPN(event.midiIndex, event.midiValue, channel, false);
                }
            }
            else
            {
                interfaceInstance->sendNRPN(event.midiIndex, event.midiValue, channel, false);
            }
        }
        break;

        case MIDI::messageType_t::nrpn14bit:
        {
            if (useOmni)
            {
                for (uint8_t channel = 1; channel <= 16; channel++)
                {
                    interfaceInstance->sendNRPN(event.midiIndex, event.midiValue, channel, true);
                }
            }
            else
            {
                interfaceInstance->sendNRPN(event.midiIndex, event.midiValue, channel, true);
            }
        }
        break;

        case MIDI::messageType_t::controlChange14bit:
        {
            if (useOmni)
            {
                for (uint8_t channel = 1; channel <= 16; channel++)
                {
                    interfaceInstance->sendControlChange14bit(event.midiIndex, event.midiValue, channel);
                }
            }
            else
            {
                interfaceInstance->sendControlChange14bit(event.midiIndex, event.midiValue, channel);
            }
        }
        break;

        case MIDI::messageType_t::systemExclusive:
        {
            if (source == Messaging::eventType_t::system)
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
    int32_t readValue = 0;
    uint8_t result    = System::Config::status_t::errorRead;

    switch (section)
    {
    case System::Config::Section::global_t::midiSettings:
    {
        auto feature = static_cast<setting_t>(index);

        switch (feature)
        {
        case setting_t::standardNoteOff:
        {
            result = _database.read(Util::Conversion::sys2DBsection(section), index, readValue) ? System::Config::status_t::ack : System::Config::status_t::errorRead;
        }
        break;

        case setting_t::runningStatus:
        case setting_t::dinEnabled:
        case setting_t::dinThruDin:
        case setting_t::dinThruUsb:
        case setting_t::usbThruDin:
        {
            if (_hwaDIN.supported())
            {
                if (!isSettingEnabled(setting_t::dinEnabled) && _hwaDIN.allocated(IO::Common::interface_t::uart))
                {
                    result = System::Config::status_t::serialPeripheralAllocatedError;
                }
                else
                {
                    result = _database.read(Util::Conversion::sys2DBsection(section), index, readValue) ? System::Config::status_t::ack : System::Config::status_t::errorRead;
                }
            }
            else
            {
                result = System::Config::status_t::errorNotSupported;
            }
        }
        break;

        default:
        {
            result = _database.read(Util::Conversion::sys2DBsection(section), index, readValue) ? System::Config::status_t::ack : System::Config::status_t::errorRead;
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
    uint8_t result            = System::Config::status_t::errorWrite;
    bool    writeToDb         = true;
    auto    dinMIDIinitAction = Common::initAction_t::asIs;
    bool    checkDINLoopback  = false;

    switch (section)
    {
    case System::Config::Section::global_t::midiSettings:
    {
        auto setting = static_cast<setting_t>(index);

        switch (setting)
        {
        case setting_t::runningStatus:
        {
            // this setting applies to din midi only
            if (_hwaDIN.supported())
            {
                if (!isSettingEnabled(setting_t::dinEnabled) && _hwaDIN.allocated(IO::Common::interface_t::uart))
                {
                    result = System::Config::status_t::serialPeripheralAllocatedError;
                }
                else
                {
                    _dinMIDI.setRunningStatusState(value);
                    result = System::Config::status_t::ack;
                }
            }
            else
            {
                result = System::Config::status_t::errorNotSupported;
            }
        }
        break;

        case setting_t::standardNoteOff:
        {
            setNoteOffMode(value ? noteOffType_t::standardNoteOff : noteOffType_t::noteOnZeroVel);
            result = System::Config::status_t::ack;
        }
        break;

        case setting_t::dinEnabled:
        {
            if (_hwaDIN.supported())
            {
                if (!isSettingEnabled(setting_t::dinEnabled) && _hwaDIN.allocated(IO::Common::interface_t::uart))
                {
                    result = System::Config::status_t::serialPeripheralAllocatedError;
                }
                else
                {
                    if (value)
                    {
                        dinMIDIinitAction = Common::initAction_t::init;
                    }
                    else
                    {
                        dinMIDIinitAction = Common::initAction_t::deInit;
                    }

                    result = System::Config::status_t::ack;
                }
            }
            else
            {
                result = System::Config::status_t::errorNotSupported;
            }
        }
        break;

        case setting_t::dinThruDin:
        {
            if (value)
            {
                _dinMIDI.registerThruInterface(_dinMIDI.transport());
            }
            else
            {
                _dinMIDI.unregisterThruInterface(_dinMIDI.transport());
            }

            result           = System::Config::status_t::ack;
            checkDINLoopback = true;
        }
        break;

        case setting_t::dinThruUsb:
        {
            if (value)
            {
                _dinMIDI.registerThruInterface(_usbMIDI.transport());
            }
            else
            {
                _dinMIDI.unregisterThruInterface(_usbMIDI.transport());
            }

            result           = System::Config::status_t::ack;
            checkDINLoopback = true;
        }
        break;

        case setting_t::usbThruDin:
        {
            if (value)
            {
                _usbMIDI.registerThruInterface(_dinMIDI.transport());
            }
            else
            {
                _usbMIDI.unregisterThruInterface(_dinMIDI.transport());
            }

            result = System::Config::status_t::ack;
        }
        break;

        case setting_t::usbThruUsb:
        {
            if (value)
            {
                _usbMIDI.registerThruInterface(_usbMIDI.transport());
            }
            else
            {
                _usbMIDI.unregisterThruInterface(_usbMIDI.transport());
            }

            result = System::Config::status_t::ack;
        }
        break;

        case setting_t::globalChannel:
        {
            if ((value < 1) || (value > MIDI_CHANNEL_OMNI))
            {
                // invalid channel
                result = System::Config::status_t::errorNewValue;
                break;
            }

            result = System::Config::status_t::ack;
        }
        break;

        default:
        {
            result = System::Config::status_t::ack;
        }
        break;
        }
    }
    break;

    default:
        return std::nullopt;
    }

    if ((result == System::Config::status_t::ack) && writeToDb)
    {
        result = _database.update(Util::Conversion::sys2DBsection(section), index, value) ? System::Config::status_t::ack : System::Config::status_t::errorWrite;

        switch (dinMIDIinitAction)
        {
        case Common::initAction_t::init:
        {
            _dinMIDI.init();
        }
        break;

        case Common::initAction_t::deInit:
        {
            _dinMIDI.deInit();
        }
        break;

        default:
            break;
        }
    }

    // no need to check this if init/deinit has been already called for DIN
    if (result && checkDINLoopback && dinMIDIinitAction == Common::initAction_t::asIs)
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