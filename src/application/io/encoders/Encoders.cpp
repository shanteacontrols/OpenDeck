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

#include "Encoders.h"
#include "io/common/Common.h"
#include "core/src/general/Timing.h"
#include "core/src/general/Helpers.h"

using namespace IO;

/// Initializes values for all encoders to their defaults.
void Encoders::init()
{
    for (int i = 0; i < MAX_NUMBER_OF_ENCODERS; i++)
        resetValue(i);
}

/// Continuously checks state of all encoders.
void Encoders::update()
{
    for (int i = 0; i < MAX_NUMBER_OF_ENCODERS; i++)
    {
        if (!_database.read(Database::Section::encoder_t::enable, i))
            continue;

        position_t encoderState = read(i, _hwa.state(i));

        //disable debounce mode if encoder isn't moving for more than
        //ENCODERS_DEBOUNCE_RESET_TIME milliseconds
        if ((core::timing::currentRunTimeMs() - _lastMovementTime[i]) > ENCODERS_DEBOUNCE_RESET_TIME)
        {
            _debounceCounter[i]   = 0;
            _debounceDirection[i] = position_t::stopped;
        }

        if (encoderState != position_t::stopped)
        {
            if (_database.read(Database::Section::encoder_t::invert, i))
            {
                if (encoderState == position_t::ccw)
                    encoderState = position_t::cw;
                else
                    encoderState = position_t::ccw;
            }

            if (_debounceCounter[i] != ENCODERS_DEBOUNCE_COUNT)
            {
                if (encoderState != _lastDirection[i])
                    _debounceCounter[i] = 0;

                _debounceCounter[i]++;

                if (_debounceCounter[i] == ENCODERS_DEBOUNCE_COUNT)
                {
                    _debounceCounter[i]   = 0;
                    _debounceDirection[i] = encoderState;
                }
            }

            uint8_t encAcceleration = _database.read(Database::Section::encoder_t::acceleration, i);

            if (encAcceleration)
            {
                //when time difference between two movements is smaller than ENCODERS_SPEED_TIMEOUT,
                //start accelerating
                if ((core::timing::currentRunTimeMs() - _lastMovementTime[i]) < ENCODERS_SPEED_TIMEOUT)
                    _encoderSpeed[i] = CONSTRAIN(_encoderSpeed[i] + _encoderSpeedChange[encAcceleration], 0, _encoderMaxAccSpeed[encAcceleration]);
                else
                    _encoderSpeed[i] = 0;
            }

            _lastDirection[i]    = encoderState;
            _lastMovementTime[i] = core::timing::currentRunTimeMs();

            if (_debounceDirection[i] != position_t::stopped)
                encoderState = _debounceDirection[i];

            uint8_t  midiID       = _database.read(Database::Section::encoder_t::midiID, i);
            uint8_t  channel      = _database.read(Database::Section::encoder_t::midiChannel, i);
            auto     type         = static_cast<type_t>(_database.read(Database::Section::encoder_t::mode, i));
            bool     validType    = true;
            uint16_t encoderValue = 0;
            uint8_t  steps        = (_encoderSpeed[i] > 0) ? _encoderSpeed[i] : 1;
            bool     use14bit     = false;

            switch (type)
            {
            case type_t::t7Fh01h:
            case type_t::t3Fh41h:
                encoderValue = _encValue[static_cast<uint8_t>(type)][static_cast<uint8_t>(encoderState)];
                break;

            case type_t::tProgramChange:
                if (encoderState == position_t::ccw)
                {
                    if (!Common::pcIncrement(channel))
                        validType = false;
                }
                else
                {
                    if (!Common::pcDecrement(channel))
                        validType = false;
                }

                encoderValue = Common::program(channel);
                break;

            case type_t::tControlChange:
            case type_t::tPitchBend:
            case type_t::tNRPN7bit:
            case type_t::tNRPN14bit:
            case type_t::tControlChange14bit:
                if ((type == type_t::tPitchBend) || (type == type_t::tNRPN14bit) || (type == type_t::tControlChange14bit))
                    use14bit = true;

                if (use14bit && (steps > 1))
                    steps <<= 2;

                if (encoderState == position_t::ccw)
                {
                    _midiValue[i] -= steps;

                    if (_midiValue[i] < 0)
                        _midiValue[i] = 0;
                }
                else
                {
                    int16_t limit = use14bit ? 16383 : 127;

                    _midiValue[i] += steps;

                    if (_midiValue[i] > limit)
                        _midiValue[i] = limit;
                }

                encoderValue = _midiValue[i];
                break;

            case type_t::tPresetChange:
                //nothing to do - valid type
                break;

            default:
                validType = false;
                break;
            }

            if (validType)
            {
                if (type == type_t::tProgramChange)
                {
                    _midi.sendProgramChange(encoderValue, channel);
                    _display.displayMIDIevent(Display::eventType_t::out, Display::event_t::programChange, midiID & 0x7F, encoderValue, channel + 1);
                }
                else if (type == type_t::tPitchBend)
                {
                    _midi.sendPitchBend(encoderValue, channel);
                    _display.displayMIDIevent(Display::eventType_t::out, Display::event_t::pitchBend, midiID & 0x7F, encoderValue, channel + 1);
                }
                else if ((type == type_t::tNRPN7bit) || (type == type_t::tNRPN14bit) || (type == type_t::tControlChange14bit))
                {
                    MIDI::Split14bit split14bit;

                    split14bit.split(midiID);

                    _midi.sendControlChange(99, split14bit.high(), channel);
                    _midi.sendControlChange(98, split14bit.low(), channel);

                    if (type == type_t::tNRPN7bit)
                    {
                        _midi.sendControlChange(6, encoderValue, channel);
                    }
                    else
                    {
                        midiID = split14bit.low();

                        split14bit.split(encoderValue);

                        if (type == type_t::tControlChange14bit)
                        {
                            if (midiID >= 96)
                                break;    //not allowed

                            _midi.sendControlChange(midiID, split14bit.high(), channel);
                            _midi.sendControlChange(midiID + 32, split14bit.low(), channel);
                        }
                        else
                        {
                            _midi.sendControlChange(6, split14bit.high(), channel);
                            _midi.sendControlChange(38, split14bit.low(), channel);
                        }
                    }

                    _display.displayMIDIevent(Display::eventType_t::out, (type == type_t::tControlChange14bit) ? Display::event_t::controlChange : Display::event_t::nrpn, midiID, encoderValue, channel + 1);
                }
                else if (type != type_t::tPresetChange)
                {
                    _midi.sendControlChange(midiID, encoderValue, channel);
                    _display.displayMIDIevent(Display::eventType_t::out, Display::event_t::controlChange, midiID & 0x7F, encoderValue, channel + 1);
                }
                else
                {
                    uint8_t preset = _database.getPreset();
                    preset += (encoderState == position_t::cw) ? 1 : -1;

                    _database.setPreset(preset);
                }
            }

            _cInfo.send(Database::block_t::encoders, i);
        }
    }
}

/// Sets the MIDI value of specified encoder to default.
void Encoders::resetValue(uint8_t encoderID)
{
    if (_database.read(Database::Section::encoder_t::mode, encoderID) == static_cast<int32_t>(type_t::tPitchBend))
        _midiValue[encoderID] = 8192;
    else
        _midiValue[encoderID] = 0;

    _lastMovementTime[encoderID]  = 0;
    _encoderSpeed[encoderID]      = 0;
    _debounceDirection[encoderID] = position_t::stopped;
    _debounceCounter[encoderID]   = 0;
    _encoderData[encoderID]       = 0;
    _encoderPulses[encoderID]     = 0;
}

void Encoders::setValue(uint8_t encoderID, uint16_t value)
{
    _midiValue[encoderID] = value;
}

/// Checks state of requested encoder.
/// param [in]: encoderID       Encoder which is being checked.
/// param [in]: pairState       A and B signal readings from encoder placed into bits 0 and 1.
/// returns: Encoder direction. See position_t.
Encoders::position_t Encoders::read(uint8_t encoderID, uint8_t pairState)
{
    position_t returnValue = position_t::stopped;
    pairState &= 0x03;

    //add new data

    bool process = true;

    //only process the data from encoder if there is a previous reading stored
    if (!BIT_READ(_encoderData[encoderID], 7))
        process = false;

    _encoderData[encoderID] <<= 2;
    _encoderData[encoderID] |= pairState;
    _encoderData[encoderID] |= 0x80;

    if (!process)
        return returnValue;

    _encoderPulses[encoderID] += _encoderLookUpTable[_encoderData[encoderID] & 0x0F];

    if (abs(_encoderPulses[encoderID]) >= _database.read(Database::Section::encoder_t::pulsesPerStep, encoderID))
    {
        returnValue = (_encoderPulses[encoderID] > 0) ? position_t::ccw : position_t::cw;
        //reset count
        _encoderPulses[encoderID] = 0;
    }

    return returnValue;
}