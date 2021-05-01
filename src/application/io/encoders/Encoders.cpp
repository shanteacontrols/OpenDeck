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

        uint8_t  numberOfReadings = 0;
        uint32_t states           = 0;

        if (!_hwa.state(i, numberOfReadings, states))
            continue;

        uint32_t currentTime = core::timing::currentRunTimeMs();

        for (uint8_t reading = 0; reading < numberOfReadings; reading++)
        {
            //take into account that there is a 1ms difference between readouts
            //when processing, newest sample has index 0
            //start from oldest reading which is in upper bits
            uint8_t  processIndex = numberOfReadings - 1 - reading;
            uint32_t sampleTime   = currentTime - (TIME_DIFF_READOUT * processIndex);

            //there are two readings per encoder
            uint8_t pairState = states >> (processIndex * 2);
            pairState &= 0x03;

            //when processing, newest sample has index 0
            processReading(i, pairState, sampleTime);
        }
    }
}

void Encoders::processReading(size_t index, uint8_t pairValue, uint32_t sampleTime)
{
    position_t encoderState = read(index, pairValue);

    if (_filter.isFiltered(index, encoderState, encoderState, sampleTime))
    {
        if (encoderState != position_t::stopped)
        {
            if (_database.read(Database::Section::encoder_t::invert, index))
            {
                if (encoderState == position_t::ccw)
                    encoderState = position_t::cw;
                else
                    encoderState = position_t::ccw;
            }

            uint8_t encAcceleration = _database.read(Database::Section::encoder_t::acceleration, index);

            if (encAcceleration)
            {
                //when time difference between two movements is smaller than ENCODERS_SPEED_TIMEOUT,
                //start accelerating
                if ((sampleTime - _filter.lastMovementTime(index)) < ENCODERS_SPEED_TIMEOUT)
                    _encoderSpeed[index] = CONSTRAIN(_encoderSpeed[index] + _encoderSpeedChange[encAcceleration], 0, _encoderMaxAccSpeed[encAcceleration]);
                else
                    _encoderSpeed[index] = 0;
            }

            uint8_t  midiID       = _database.read(Database::Section::encoder_t::midiID, index);
            uint8_t  channel      = _database.read(Database::Section::encoder_t::midiChannel, index);
            auto     type         = static_cast<type_t>(_database.read(Database::Section::encoder_t::mode, index));
            bool     validType    = true;
            uint16_t encoderValue = 0;
            uint8_t  steps        = (_encoderSpeed[index] > 0) ? _encoderSpeed[index] : 1;
            bool     use14bit     = false;

            switch (type)
            {
            case type_t::t7Fh01h:
            case type_t::t3Fh41h:
            {
                encoderValue = _encValue[static_cast<uint8_t>(type)][static_cast<uint8_t>(encoderState)];
            }
            break;

            case type_t::tProgramChange:
            {
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
            }
            break;

            case type_t::tControlChange:
            case type_t::tPitchBend:
            case type_t::tNRPN7bit:
            case type_t::tNRPN14bit:
            case type_t::tControlChange14bit:
            {
                if ((type == type_t::tPitchBend) || (type == type_t::tNRPN14bit) || (type == type_t::tControlChange14bit))
                    use14bit = true;

                if (use14bit && (steps > 1))
                    steps <<= 2;

                if (encoderState == position_t::ccw)
                {
                    _midiValue[index] -= steps;

                    if (_midiValue[index] < 0)
                        _midiValue[index] = 0;
                }
                else
                {
                    int16_t limit = use14bit ? 16383 : 127;

                    _midiValue[index] += steps;

                    if (_midiValue[index] > limit)
                        _midiValue[index] = limit;
                }

                encoderValue = _midiValue[index];
            }
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
                            if (midiID < 96)
                            {
                                _midi.sendControlChange(midiID, split14bit.high(), channel);
                                _midi.sendControlChange(midiID + 32, split14bit.low(), channel);
                            }
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

            _cInfo.send(Database::block_t::encoders, index);
        }
    }
}

/// Sets the MIDI value of specified encoder to default.
void Encoders::resetValue(size_t index)
{
    if (_database.read(Database::Section::encoder_t::mode, index) == static_cast<int32_t>(type_t::tPitchBend))
        _midiValue[index] = 8192;
    else
        _midiValue[index] = 0;

    _filter.reset(index);
    _encoderSpeed[index]  = 0;
    _encoderData[index]   = 0;
    _encoderPulses[index] = 0;
}

void Encoders::setValue(size_t index, uint16_t value)
{
    _midiValue[index] = value;
}

/// Checks state of requested encoder.
/// param [in]: index           Encoder which is being checked.
/// param [in]: pairState       A and B signal readings from encoder placed into bits 0 and 1.
/// returns: Encoder direction. See position_t.
Encoders::position_t Encoders::read(size_t index, uint8_t pairState)
{
    position_t returnValue = position_t::stopped;
    pairState &= 0x03;

    //add new data

    bool process = true;

    //only process the data from encoder if there is a previous reading stored
    if (!BIT_READ(_encoderData[index], 7))
        process = false;

    _encoderData[index] <<= 2;
    _encoderData[index] |= pairState;
    _encoderData[index] |= 0x80;

    if (!process)
        return returnValue;

    _encoderPulses[index] += _encoderLookUpTable[_encoderData[index] & 0x0F];

    if (abs(_encoderPulses[index]) >= _database.read(Database::Section::encoder_t::pulsesPerStep, index))
    {
        returnValue = (_encoderPulses[index] > 0) ? position_t::ccw : position_t::cw;
        //reset count
        _encoderPulses[index] = 0;
    }

    return returnValue;
}