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
#include "util/messaging/Messaging.h"
#include "util/configurable/Configurable.h"

using namespace IO;

Protocol::MIDI::MIDI(HWA& hwa, Database& database)
    : ::MIDI(_hwaInternal)
    , _hwa(hwa)
    , _hwaInternal(*this)
    , _database(database)
{
    Dispatcher.listen(Util::MessageDispatcher::messageSource_t::analog,
                      Util::MessageDispatcher::listenType_t::nonFwd,
                      [this](const Util::MessageDispatcher::message_t& dispatchMessage) {
                          sendMIDI(dispatchMessage);
                      });

    Dispatcher.listen(Util::MessageDispatcher::messageSource_t::buttons,
                      Util::MessageDispatcher::listenType_t::nonFwd,
                      [this](const Util::MessageDispatcher::message_t& dispatchMessage) {
                          sendMIDI(dispatchMessage);
                      });

    Dispatcher.listen(Util::MessageDispatcher::messageSource_t::encoders,
                      Util::MessageDispatcher::listenType_t::nonFwd,
                      [this](const Util::MessageDispatcher::message_t& dispatchMessage) {
                          sendMIDI(dispatchMessage);
                      });

    Dispatcher.listen(Util::MessageDispatcher::messageSource_t::touchscreenButton,
                      Util::MessageDispatcher::listenType_t::nonFwd,
                      [this](const Util::MessageDispatcher::message_t& dispatchMessage) {
                          sendMIDI(dispatchMessage);
                      });

    Dispatcher.listen(Util::MessageDispatcher::messageSource_t::touchscreenAnalog,
                      Util::MessageDispatcher::listenType_t::nonFwd,
                      [this](const Util::MessageDispatcher::message_t& dispatchMessage) {
                          sendMIDI(dispatchMessage);
                      });

    Dispatcher.listen(Util::MessageDispatcher::messageSource_t::system,
                      Util::MessageDispatcher::listenType_t::nonFwd,
                      [this](const Util::MessageDispatcher::message_t& dispatchMessage) {
                          switch (dispatchMessage.componentIndex)
                          {
                          case static_cast<uint16_t>(Util::MessageDispatcher::systemMessages_t::sysExResponse):
                          {
                              sendMIDI(dispatchMessage);
                          }
                          break;

                          default:
                              break;
                          }
                      });

    Dispatcher.listen(Util::MessageDispatcher::messageSource_t::preset,
                      Util::MessageDispatcher::listenType_t::all,
                      [this](const Util::MessageDispatcher::message_t& dispatchMessage) {
                          if (!::MIDI::init(MIDI::interface_t::din))
                          {
                              ::MIDI::deInit(MIDI::interface_t::din);
                          }
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
    if (!::MIDI::init(MIDI::interface_t::usb))
    {
        return false;
    }

    ::MIDI::setInputChannel(::MIDI::MIDI_CHANNEL_OMNI);
    ::MIDI::setNoteOffMode(isFeatureEnabled(feature_t::standardNoteOff) ? MIDI::noteOffType_t::standardNoteOff : MIDI::noteOffType_t::noteOnZeroVel);
    ::MIDI::setRunningStatusState(isFeatureEnabled(feature_t::runningStatus));
    ::MIDI::setChannelSendZeroStart(true);
    ::MIDI::useRecursiveParsing(true);

    if (isFeatureEnabled(feature_t::dinEnabled))
    {
        ::MIDI::init(MIDI::interface_t::din);
    }
    else
    {
        ::MIDI::deInit(MIDI::interface_t::din);
    }

    return true;
}

bool Protocol::MIDI::deInit()
{
    if (!::MIDI::deInit(MIDI::interface_t::din))
    {
        return false;
    }

    if (!::MIDI::deInit(MIDI::interface_t::usb))
    {
        return false;
    }

    return true;
}

void Protocol::MIDI::read()
{
    auto processMessage = [&](MIDI::interface_t interface) {
        Util::MessageDispatcher::message_t dispatchMessage;

        dispatchMessage.componentIndex = 0;
        dispatchMessage.midiChannel    = getChannel(interface);
        dispatchMessage.midiIndex      = getData1(interface);
        dispatchMessage.midiValue      = getData2(interface);
        dispatchMessage.message        = getType(interface);

        switch (dispatchMessage.message)
        {
        case MIDI::messageType_t::systemExclusive:
        {
            // process sysex messages only from usb interface
            if (interface == MIDI::interface_t::usb)
            {
                dispatchMessage.sysEx       = getSysExArray(interface);
                dispatchMessage.sysExLength = getSysExArrayLength(interface);
            }
        }
        break;

        case MIDI::messageType_t::programChange:
        {
            Common::setProgram(dispatchMessage.midiChannel, dispatchMessage.midiIndex);
            _database.setPreset(dispatchMessage.midiIndex);
        }
        break;

        case MIDI::messageType_t::noteOff:
        {
            dispatchMessage.midiValue = 0;
        }
        break;

        default:
            break;
        }

        Dispatcher.notify(Util::MessageDispatcher::messageSource_t::midiIn,
                          dispatchMessage,
                          Util::MessageDispatcher::listenType_t::nonFwd);
    };

    if (
        isFeatureEnabled(feature_t::dinEnabled) &&
        isFeatureEnabled(feature_t::passToDIN))
    {
        // pass the message to din
        while (::MIDI::read(MIDI::interface_t::usb, MIDI::filterMode_t::fullDIN))
        {
            processMessage(MIDI::interface_t::usb);
        }
    }
    else
    {
        while (::MIDI::read(MIDI::interface_t::usb))
        {
            processMessage(MIDI::interface_t::usb);
        }
    }

    if (isFeatureEnabled(feature_t::dinEnabled))
    {
        if (isFeatureEnabled(feature_t::mergeEnabled))
        {
            switch (mergeType())
            {
            case mergeType_t::DINtoUSB:
            {
                // dump everything from DIN MIDI in to USB MIDI out
                while (::MIDI::read(MIDI::interface_t::din, MIDI::filterMode_t::fullUSB))
                {
                    ;
                }
            }
            break;

                // case mergeType_t::DINtoDIN:
                // loopback is automatically configured here
                // break;

            default:
                break;
            }
        }
        else
        {
            while (::MIDI::read(MIDI::interface_t::din))
            {
                processMessage(MIDI::interface_t::din);
            }
        }
    }
}

void Protocol::MIDI::sendMIDI(const Util::MessageDispatcher::message_t& message)
{
    switch (message.message)
    {
    case ::MIDI::messageType_t::noteOff:
    {
        sendNoteOff(message.midiIndex, message.midiValue, message.midiChannel);
    }
    break;

    case ::MIDI::messageType_t::noteOn:
    {
        if (!message.midiValue && (getNoteOffMode() == ::MIDI::noteOffType_t::standardNoteOff))
        {
            sendNoteOff(message.midiIndex, message.midiValue, message.midiChannel);
        }
        else
        {
            sendNoteOn(message.midiIndex, message.midiValue, message.midiChannel);
        }
    }
    break;

    case ::MIDI::messageType_t::controlChange:
    {
        sendControlChange(message.midiIndex, message.midiValue, message.midiChannel);
    }
    break;

    case ::MIDI::messageType_t::programChange:
    {
        sendProgramChange(message.midiIndex, message.midiChannel);
    }
    break;

    case ::MIDI::messageType_t::afterTouchChannel:
    {
        sendAfterTouch(message.midiValue, message.midiChannel);
    }
    break;

    case ::MIDI::messageType_t::afterTouchPoly:
    {
        sendAfterTouch(message.midiValue, message.midiChannel, message.midiIndex);
    }
    break;

    case ::MIDI::messageType_t::pitchBend:
    {
        sendPitchBend(message.midiValue, message.midiChannel);
    }
    break;

    case ::MIDI::messageType_t::sysRealTimeClock:
    {
        sendRealTime(message.message);
    }
    break;

    case ::MIDI::messageType_t::sysRealTimeStart:
    {
        sendRealTime(message.message);
    }
    break;

    case ::MIDI::messageType_t::sysRealTimeContinue:
    {
        sendRealTime(message.message);
    }
    break;

    case ::MIDI::messageType_t::sysRealTimeStop:
    {
        sendRealTime(message.message);
    }
    break;

    case ::MIDI::messageType_t::sysRealTimeActiveSensing:
    {
        sendRealTime(message.message);
    }
    break;

    case ::MIDI::messageType_t::sysRealTimeSystemReset:
    {
        sendRealTime(message.message);
    }
    break;

    case ::MIDI::messageType_t::mmcPlay:
    {
        sendMMC(message.midiIndex, message.message);
    }
    break;

    case ::MIDI::messageType_t::mmcStop:
    {
        sendMMC(message.midiIndex, message.message);
    }
    break;

    case ::MIDI::messageType_t::mmcPause:
    {
        sendMMC(message.midiIndex, message.message);
    }
    break;

    case ::MIDI::messageType_t::mmcRecordStart:
    {
        sendMMC(message.midiIndex, message.message);
    }
    break;

    case ::MIDI::messageType_t::mmcRecordStop:
    {
        sendMMC(message.midiIndex, message.message);
    }
    break;

    case ::MIDI::messageType_t::nrpn7bit:
    {
        sendNRPN(message.midiIndex, message.midiValue, message.midiChannel, false);
    }
    break;

    case ::MIDI::messageType_t::nrpn14bit:
    {
        sendNRPN(message.midiIndex, message.midiValue, message.midiChannel, true);
    }
    break;

    case ::MIDI::messageType_t::controlChange14bit:
    {
        sendControlChange14bit(message.midiIndex, message.midiValue, message.midiChannel);
    }
    break;

    case ::MIDI::messageType_t::systemExclusive:
    {
        // never send system sysex through DIN MIDI
        sendSysEx(message.sysExLength, message.sysEx, true, MIDI::interface_t::usb);
    }
    break;

    default:
        break;
    }
}

bool Protocol::MIDI::isFeatureEnabled(feature_t feature)
{
    return _database.read(Database::Section::global_t::midiFeatures, feature);
};

Protocol::MIDI::mergeType_t Protocol::MIDI::mergeType()
{
    return static_cast<mergeType_t>(_database.read(Database::Section::global_t::midiMerge, mergeSetting_t::mergeType));
}

bool Protocol::MIDI::HWAInternal::init(MIDI::interface_t interface)
{
    if (interface == ::MIDI::interface_t::usb)
    {
        return _midi._hwa.init(interface);
    }

    if (_midi.isFeatureEnabled(feature_t::dinEnabled))
    {
        auto mergeType    = static_cast<MIDI::mergeType_t>(_midi._database.read(Database::Section::global_t::midiMerge, MIDI::mergeSetting_t::mergeType));
        bool mergeEnabled = _midi._database.read(Database::Section::global_t::midiFeatures, MIDI::feature_t::mergeEnabled);

        bool loopback = mergeType == MIDI::mergeType_t::DINtoDIN && mergeEnabled;

        if (_dinMIDIenabled && (loopback == _dinMIDIloopbackEnabled))
        {
            return true;    // nothing do do
        }

        if (!_dinMIDIenabled)
        {
            if (_midi._hwa.init(interface))
            {
                _midi._hwa.setDINLoopback(loopback);
                _dinMIDIenabled         = true;
                _dinMIDIloopbackEnabled = loopback;

                return true;
            }
        }
        else
        {
            if (loopback != _dinMIDIloopbackEnabled)
            {
                // only the loopback parameter has changed
                _dinMIDIloopbackEnabled = loopback;
                _midi._hwa.setDINLoopback(loopback);
                return true;
            }
        }
    }

    return false;
}

bool Protocol::MIDI::HWAInternal::deInit(MIDI::interface_t interface)
{
    if (interface == MIDI::interface_t::din)
    {
        if (!_dinMIDIenabled)
        {
            return true;    // nothing to do
        }

        if (_midi._hwa.deInit(interface))
        {
            _dinMIDIenabled         = false;
            _dinMIDIloopbackEnabled = false;
            return true;
        }
    }
    else
    {
        return _midi._hwa.deInit(interface);
    }

    return false;
}

bool Protocol::MIDI::HWAInternal::dinRead(uint8_t& value)
{
    return _midi._hwa.dinRead(value);
}

bool Protocol::MIDI::HWAInternal::dinWrite(uint8_t value)
{
    return _midi._hwa.dinWrite(value);
}

bool Protocol::MIDI::HWAInternal::usbRead(MIDI::USBMIDIpacket_t& USBMIDIpacket)
{
    return _midi._hwa.usbRead(USBMIDIpacket);
}

bool Protocol::MIDI::HWAInternal::usbWrite(MIDI::USBMIDIpacket_t& USBMIDIpacket)
{
    return _midi._hwa.usbWrite(USBMIDIpacket);
}

std::optional<uint8_t> Protocol::MIDI::sysConfigGet(System::Config::Section::global_t section, size_t index, uint16_t& value)
{
    int32_t readValue = 0;
    uint8_t result    = System::Config::status_t::errorRead;

    switch (section)
    {
    case System::Config::Section::global_t::midiFeatures:
    {
        if (index == static_cast<size_t>(feature_t::standardNoteOff))
        {
            result = _database.read(Util::Conversion::sys2DBsection(section), index, readValue) ? System::Config::status_t::ack : System::Config::status_t::errorRead;
        }
        else
        {
            if (_hwa.dinSupported())
            {
                if (!isFeatureEnabled(feature_t::dinEnabled) && _hwa.allocated(IO::Common::interface_t::uart))
                {
                    return System::Config::status_t::serialPeripheralAllocatedError;
                }

                result = _database.read(Util::Conversion::sys2DBsection(section), index, readValue) ? System::Config::status_t::ack : System::Config::status_t::errorRead;
            }
            else
            {
                result = System::Config::status_t::errorNotSupported;
            }
        }
    }
    break;

    case System::Config::Section::global_t::midiMerge:
    {
        if (_hwa.dinSupported())
        {
            if (!isFeatureEnabled(feature_t::dinEnabled) && _hwa.allocated(IO::Common::interface_t::uart))
            {
                return System::Config::status_t::serialPeripheralAllocatedError;
            }

            result = _database.read(Util::Conversion::sys2DBsection(section), index, readValue) ? System::Config::status_t::ack : System::Config::status_t::errorRead;
        }
        else
        {
            result = System::Config::status_t::errorNotSupported;
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

    switch (section)
    {
    case System::Config::Section::global_t::midiFeatures:
    {
        auto feature = static_cast<Protocol::MIDI::feature_t>(index);

        switch (feature)
        {
        case feature_t::runningStatus:
        {
            if (_hwa.dinSupported())
            {
                if (!isFeatureEnabled(feature_t::dinEnabled) && _hwa.allocated(IO::Common::interface_t::uart))
                {
                    result = System::Config::status_t::serialPeripheralAllocatedError;
                }
                else
                {
                    setRunningStatusState(value);
                    result = System::Config::status_t::ack;
                }
            }
            else
            {
                result = System::Config::status_t::errorNotSupported;
            }
        }
        break;

        case feature_t::standardNoteOff:
        {
            if (value)
            {
                setNoteOffMode(::MIDI::noteOffType_t::standardNoteOff);
            }
            else
            {
                setNoteOffMode(::MIDI::noteOffType_t::noteOnZeroVel);
            }

            result = System::Config::status_t::ack;
        }
        break;

        case feature_t::dinEnabled:
        {
            if (_hwa.dinSupported())
            {
                if (!isFeatureEnabled(feature_t::dinEnabled) && _hwa.allocated(IO::Common::interface_t::uart))
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

        case feature_t::mergeEnabled:
        {
            if (_hwa.dinSupported())
            {
                if (!isFeatureEnabled(feature_t::dinEnabled) && _hwa.allocated(IO::Common::interface_t::uart))
                {
                    result = System::Config::status_t::serialPeripheralAllocatedError;
                }
                else
                {
                    result = System::Config::status_t::ack;

                    if (isFeatureEnabled(feature_t::dinEnabled))
                    {
                        dinMIDIinitAction = Common::initAction_t::init;
                    }
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
            result = System::Config::status_t::ack;
        }
        break;
        }
    }
    break;

    case System::Config::Section::global_t::midiMerge:
    {
        if (_hwa.dinSupported())
        {
            if (!isFeatureEnabled(feature_t::dinEnabled) && _hwa.allocated(IO::Common::interface_t::uart))
            {
                result = System::Config::status_t::serialPeripheralAllocatedError;
            }
            else
            {
                auto mergeParam = static_cast<mergeSetting_t>(index);

                switch (mergeParam)
                {
                case mergeSetting_t::mergeType:
                {
                    if ((value >= 0) && (value < static_cast<size_t>(mergeType_t::AMOUNT)))
                    {
                        result = System::Config::status_t::ack;

                        if (isFeatureEnabled(feature_t::dinEnabled))
                        {
                            dinMIDIinitAction = Common::initAction_t::init;
                        }
                    }
                    else
                    {
                        result = System::Config::status_t::errorNotSupported;
                    }
                }
                break;

                case mergeSetting_t::mergeUSBchannel:
                case mergeSetting_t::mergeDINchannel:
                {
                    // unused for now
                    writeToDb = false;
                    result    = System::Config::status_t::ack;
                }
                break;

                default:
                    break;
                }
            }
        }
        else
        {
            result = System::Config::status_t::errorNotSupported;
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
            ::MIDI::init(MIDI::interface_t::din);
        }
        break;

        case Common::initAction_t::deInit:
        {
            ::MIDI::deInit(MIDI::interface_t::din);
        }
        break;

        default:
            break;
        }
    }

    return result;
}