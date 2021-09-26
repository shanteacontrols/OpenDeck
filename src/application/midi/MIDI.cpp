/*

Copyright 2015-2021 Igor Petrovic

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

void Protocol::MIDI::sendMIDI(const MessageDispatcher::message_t& message)
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
            sendNoteOff(message.midiIndex, message.midiValue, message.midiChannel);
        else
            sendNoteOn(message.midiIndex, message.midiValue, message.midiChannel);
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

    default:
        break;
    }
}