/*

Copyright 2015-2020 Igor Petrovic

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
#include "board/Board.h"
#include "interface/CInfo.h"
#include "core/src/general/Timing.h"
#include "core/src/general/Helpers.h"

using namespace Interface::digital::input;

///
/// \brief Initializes values for all encoders to their defaults.
///
void Encoders::init()
{
    for (int i = 0; i < MAX_NUMBER_OF_ENCODERS; i++)
        resetValue(i);
}

///
/// \brief Continuously checks state of all encoders.
///
void Encoders::update()
{
    for (int i = 0; i < MAX_NUMBER_OF_ENCODERS; i++)
    {
        if (!database.read(DB_BLOCK_ENCODERS, dbSection_encoders_enable, i))
            continue;

        position_t encoderState = read(i, Board::io::getEncoderPairState(i));

        //disable debounce mode if encoder isn't moving for more than
        //ENCODERS_DEBOUNCE_RESET_TIME milliseconds
        if ((core::timing::currentRunTimeMs() - lastMovementTime[i]) > ENCODERS_DEBOUNCE_RESET_TIME)
        {
            debounceCounter[i]   = 0;
            debounceDirection[i] = position_t::stopped;
        }

        if (encoderState != position_t::stopped)
        {
            if (database.read(DB_BLOCK_ENCODERS, dbSection_encoders_invert, i))
            {
                if (encoderState == position_t::ccw)
                    encoderState = position_t::cw;
                else
                    encoderState = position_t::ccw;
            }

            if (debounceCounter[i] != ENCODERS_DEBOUNCE_COUNT)
            {
                if (encoderState != lastDirection[i])
                    debounceCounter[i] = 0;

                debounceCounter[i]++;

                if (debounceCounter[i] == ENCODERS_DEBOUNCE_COUNT)
                {
                    debounceCounter[i]   = 0;
                    debounceDirection[i] = encoderState;
                }
            }

            uint8_t encAcceleration = database.read(DB_BLOCK_ENCODERS, dbSection_encoders_acceleration, i);

            if (encAcceleration)
            {
                //when time difference between two movements is smaller than ENCODERS_SPEED_TIMEOUT,
                //start accelerating
                if ((core::timing::currentRunTimeMs() - lastMovementTime[i]) < ENCODERS_SPEED_TIMEOUT)
                    encoderSpeed[i] = CONSTRAIN(encoderSpeed[i] + encoderSpeedChange[encAcceleration], 0, encoderMaxAccSpeed[encAcceleration]);
                else
                    encoderSpeed[i] = 0;
            }

            lastDirection[i]    = encoderState;
            lastMovementTime[i] = core::timing::currentRunTimeMs();

            if (debounceDirection[i] != position_t::stopped)
                encoderState = debounceDirection[i];

            uint8_t  midiID       = database.read(DB_BLOCK_ENCODERS, dbSection_encoders_midiID, i);
            uint8_t  channel      = database.read(DB_BLOCK_ENCODERS, dbSection_encoders_midiChannel, i);
            auto     type         = static_cast<type_t>(database.read(DB_BLOCK_ENCODERS, dbSection_encoders_mode, i));
            bool     validType    = true;
            uint16_t encoderValue = 0;
            uint8_t  steps        = (encoderSpeed[i] > 0) ? encoderSpeed[i] : 1;
            bool     use14bit     = false;

            MIDI::encDec_14bit_t encDec_14bit;

            switch (type)
            {
            case type_t::t7Fh01h:
            case type_t::t3Fh41h:
                encoderValue = encValue[static_cast<uint8_t>(type)][static_cast<uint8_t>(encoderState)];
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
                    midiValue[i] -= steps;

                    if (midiValue[i] < 0)
                        midiValue[i] = 0;
                }
                else
                {
                    int16_t limit = use14bit ? 16383 : 127;

                    midiValue[i] += steps;

                    if (midiValue[i] > limit)
                        midiValue[i] = limit;
                }

                encoderValue = midiValue[i];
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
                    midi.sendProgramChange(encoderValue, channel);
#ifdef DISPLAY_SUPPORTED
                    display.displayMIDIevent(Display::eventType_t::out, Display::event_t::programChange, midiID & 0x7F, encoderValue, channel + 1);
#endif
                }
                else if (type == type_t::tPitchBend)
                {
                    midi.sendPitchBend(encoderValue, channel);
#ifdef DISPLAY_SUPPORTED
                    display.displayMIDIevent(Display::eventType_t::out, Display::event_t::pitchBend, midiID & 0x7F, encoderValue, channel + 1);
#endif
                }
                else if ((type == type_t::tNRPN7bit) || (type == type_t::tNRPN14bit) || (type == type_t::tControlChange14bit))
                {
                    encDec_14bit.value = midiID;
                    encDec_14bit.split14bit();

                    midi.sendControlChange(99, encDec_14bit.high, channel);
                    midi.sendControlChange(98, encDec_14bit.low, channel);

                    if (type == type_t::tNRPN7bit)
                    {
                        midi.sendControlChange(6, encoderValue, channel);
                    }
                    else
                    {
                        midiID = encDec_14bit.low;

                        encDec_14bit.value = encoderValue;
                        encDec_14bit.split14bit();

                        if (type == type_t::tControlChange14bit)
                        {
                            if (midiID >= 96)
                                break;    //not allowed

                            midi.sendControlChange(midiID, encDec_14bit.high, channel);
                            midi.sendControlChange(midiID + 32, encDec_14bit.low, channel);
                        }
                        else
                        {
                            midi.sendControlChange(6, encDec_14bit.high, channel);
                            midi.sendControlChange(38, encDec_14bit.low, channel);
                        }
                    }

#ifdef DISPLAY_SUPPORTED
                    display.displayMIDIevent(Display::eventType_t::out, (type == type_t::tControlChange14bit) ? Display::event_t::controlChange : Display::event_t::nrpn, midiID, encoderValue, channel + 1);
#endif
                }
                else if (type != type_t::tPresetChange)
                {
                    midi.sendControlChange(midiID, encoderValue, channel);
#ifdef DISPLAY_SUPPORTED
                    display.displayMIDIevent(Display::eventType_t::out, Display::event_t::controlChange, midiID & 0x7F, encoderValue, channel + 1);
#endif
                }
                else
                {
                    uint8_t preset = database.getPreset();
                    preset += (encoderState == position_t::cw) ? 1 : -1;

                    database.setPreset(preset);
                }
            }

            cInfo.send(DB_BLOCK_ENCODERS, i);
        }
    }
}

///
/// \brief Sets the MIDI value of specified encoder to default.
///
void Encoders::resetValue(uint8_t encoderID)
{
    if (database.read(DB_BLOCK_ENCODERS, dbSection_encoders_mode, encoderID) == static_cast<int32_t>(type_t::tPitchBend))
        midiValue[encoderID] = 8192;
    else
        midiValue[encoderID] = 0;

    lastMovementTime[encoderID]  = 0;
    encoderSpeed[encoderID]      = 0;
    debounceDirection[encoderID] = position_t::stopped;
    debounceCounter[encoderID]   = 0;
    encoderData[encoderID]       = 0;
    encoderPulses[encoderID]     = 0;
}

void Encoders::setValue(uint8_t encoderID, uint16_t value)
{
    midiValue[encoderID] = value;
}

///
/// \brief Checks state of requested encoder.
/// @param [in] encoderID       Encoder which is being checked.
/// @param [in] pairState       A and B signal readings from encoder placed into bits 0 and 1.
/// \returns Encoder direction. See position_t.
///
Encoders::position_t Encoders::read(uint8_t encoderID, uint8_t pairState)
{
    position_t returnValue = position_t::stopped;
    pairState &= 0x03;

    //add new data

    bool process = true;

    //only process the data from encoder if there is a previous reading stored
    if (!BIT_READ(encoderData[encoderID], 7))
        process = false;

    encoderData[encoderID] <<= 2;
    encoderData[encoderID] |= pairState;
    encoderData[encoderID] |= 0x80;

    if (!process)
        return returnValue;

    encoderPulses[encoderID] += encoderLookUpTable[encoderData[encoderID] & 0x0F];

    if (abs(encoderPulses[encoderID]) >= database.read(DB_BLOCK_ENCODERS, dbSection_encoders_pulsesPerStep, encoderID))
    {
        returnValue = (encoderPulses[encoderID] > 0) ? position_t::ccw : position_t::cw;
        //reset count
        encoderPulses[encoderID] = 0;
    }

    return returnValue;
}