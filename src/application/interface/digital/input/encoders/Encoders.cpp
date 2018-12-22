/*

Copyright 2015-2018 Igor Petrovic

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

#include <stdlib.h>
#include "Encoders.h"
#include "interface/digital/input/Common.h"
#include "interface/CInfo.h"
#include "core/src/general/Timing.h"
#include "core/src/general/Misc.h"

///
/// \brief Initializes values for all encoders to their defaults.
///
void Encoders::init()
{
    for (int i=0; i<MAX_NUMBER_OF_ENCODERS; i++)
        resetValue(i);
}

///
/// \brief Continuously checks state of all encoders.
///
void Encoders::update()
{
    for (int i=0; i<MAX_NUMBER_OF_ENCODERS; i++)
    {
        if (!database.read(DB_BLOCK_ENCODERS, dbSection_encoders_enable, i))
            continue;

        encoderPosition_t encoderState = Board::getEncoderState(i, database.read(DB_BLOCK_ENCODERS, dbSection_encoders_pulsesPerStep, i));

        //disable debounce mode if encoder isn't moving for more than
        //DEBOUNCE_RESET_TIME milliseconds
        if ((rTimeMs() - lastMovementTime[i]) > DEBOUNCE_RESET_TIME)
        {
            debounceCounter[i] = 0;
            debounceDirection[i] = encStopped;
        }

        if (encoderState != encStopped)
        {
            if (database.read(DB_BLOCK_ENCODERS, dbSection_encoders_invert, i))
            {
                if (encoderState == encMoveLeft)
                    encoderState = encMoveRight;
                else
                    encoderState = encMoveLeft;
            }

            if (debounceCounter[i] != ENCODER_DEBOUNCE_COUNT)
            {
                if (encoderState != lastDirection[i])
                    debounceCounter[i] = 0;

                debounceCounter[i]++;

                if (debounceCounter[i] == ENCODER_DEBOUNCE_COUNT)
                {
                    debounceCounter[i] = 0;
                    debounceDirection[i] = encoderState;
                }
            }

            if (database.read(DB_BLOCK_ENCODERS, dbSection_encoders_acceleration, i))
            {
                //when time difference between two movements is smaller than SPEED_TIMEOUT,
                //start accelerating
                if ((rTimeMs() - lastMovementTime[i]) < SPEED_TIMEOUT)
                    encoderSpeed[i] = CONSTRAIN(encoderSpeed[i]+ENCODER_SPEED_CHANGE, 0, ENCODER_MAX_SPEED);
                else
                    encoderSpeed[i] = 0;
            }

            lastDirection[i] = encoderState;
            lastMovementTime[i] = rTimeMs();

            if (debounceDirection[i] != encStopped)
                encoderState = debounceDirection[i];

            uint8_t midiID = database.read(DB_BLOCK_ENCODERS, dbSection_encoders_midiID, i);
            uint8_t channel = database.read(DB_BLOCK_ENCODERS, dbSection_encoders_midiChannel, i);
            encoderType_t type = static_cast<encoderType_t>(database.read(DB_BLOCK_ENCODERS, dbSection_encoders_mode, i));
            bool validType = true;
            uint16_t encoderValue;
            uint8_t steps = (encoderSpeed[i] > 0) ? encoderSpeed[i] : 1;
            encDec_14bit_t encDec_14bit;

            switch(type)
            {
                case encType7Fh01h:
                case encType3Fh41h:
                encoderValue = encValue[static_cast<uint8_t>(type)][static_cast<uint8_t>(encoderState)];
                break;

                case encTypePC:
                if (encoderState == encMoveLeft)
                {
                    if (digitalInputCommon::detail::lastPCvalue[channel] < 127)
                        digitalInputCommon::detail::lastPCvalue[channel]++;
                }
                else
                {
                    if (digitalInputCommon::detail::lastPCvalue[channel] > 0)
                        digitalInputCommon::detail::lastPCvalue[channel]--;
                }

                encoderValue = digitalInputCommon::detail::lastPCvalue[channel];
                break;

                case encTypeCC:
                case encTypePitchBend:
                case encTypeNRPN7bit:
                case encTypeNRPN14bit:
                if (((type == encTypePitchBend) || (type == encTypeNRPN14bit)) && (steps > 1))
                    steps <<= 2;

                if (encoderState == encMoveLeft)
                {
                    midiValue[i] -= steps;

                    if (midiValue[i] < 0)
                        midiValue[i] = 0;
                }
                else
                {
                    int16_t limit = ((type == encTypeCC) || (type == encTypeNRPN7bit)) ? 127 : 16383;

                    midiValue[i] += steps;

                    if (midiValue[i] > limit)
                        midiValue[i] = limit;
                }

                encoderValue = midiValue[i];
                break;

                default:
                validType = false;
                break;
            }

            if (validType)
            {
                if (type == encTypePC)
                {
                    midi.sendProgramChange(encoderValue, channel);
                    #ifdef DISPLAY_SUPPORTED
                    display.displayMIDIevent(displayEventOut, midiMessageProgramChange_display, midiID & 0x7F, encoderValue, channel+1);
                    #endif
                }
                else if (type == encTypePitchBend)
                {
                    midi.sendPitchBend(encoderValue, channel);
                    #ifdef DISPLAY_SUPPORTED
                    display.displayMIDIevent(displayEventOut, midiMessagePitchBend_display, midiID & 0x7F, encoderValue, channel+1);
                    #endif
                }
                else if ((type == encTypeNRPN7bit) || (type == encTypeNRPN14bit))
                {
                    encDec_14bit.value = midiID;
                    encDec_14bit.split14bit();
                    midi.sendControlChange(99, encDec_14bit.high, channel);
                    midi.sendControlChange(98, encDec_14bit.low, channel);

                    if (type == encTypeNRPN7bit)
                    {
                        midi.sendControlChange(6, encoderValue, channel);
                    }
                    else
                    {
                        //use mapRange_uint32 to avoid overflow issues
                        encDec_14bit.value = encoderValue;
                        encDec_14bit.split14bit();

                        midi.sendControlChange(6, encDec_14bit.high, channel);
                        midi.sendControlChange(38, encDec_14bit.low, channel);
                    }

                    #ifdef DISPLAY_SUPPORTED
                    display.displayMIDIevent(displayEventOut, midiMessageNRPN_display, midiID, encoderValue, channel+1);
                    #endif
                }
                else if (type != encTypePresetChange)
                {
                    midi.sendControlChange(midiID, encoderValue, channel);
                    #ifdef DISPLAY_SUPPORTED
                    display.displayMIDIevent(displayEventOut, midiMessageControlChange_display, midiID & 0x7F, encoderValue, channel+1);
                    #endif
                }
                else
                {
                    uint8_t preset = database.getPreset();
                    preset += (encoderState == encMoveRight) ? 1 : -1;

                    #ifdef LEDS_SUPPORTED
                    if (database.setPreset(preset))
                    {
                        leds.midiToState(midiMessageProgramChange, preset, 0, 0, true);

                        #ifdef DISPLAY_SUPPORTED
                        display.displayMIDIevent(displayEventIn, messagePresetChange_display, preset, 0, 0);
                        #endif
                    }
                    #endif
                }
            }

            if (cinfoHandler != nullptr)
                (*cinfoHandler)(DB_BLOCK_ENCODERS, i);
        }
    }
}

///
/// \brief Sets the MIDI value of specified encoder to default.
///
void Encoders::resetValue(uint8_t encoderID)
{
    if (database.read(DB_BLOCK_ENCODERS, dbSection_encoders_mode, encoderID) == encTypePitchBend)
        midiValue[encoderID] = 8192;
    else
        midiValue[encoderID] = 0;
}