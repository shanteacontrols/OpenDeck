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

///
/// \brief Continuously checks state of all encoders.
///
void Encoders::update()
{
    uint8_t encoderValue;

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

            lastDirection[i] = encoderState;
            lastMovementTime[i] = rTimeMs();

            if (debounceDirection[i] != encStopped)
                encoderState = debounceDirection[i];

            uint8_t midiID = database.read(DB_BLOCK_ENCODERS, dbSection_encoders_midiID, i);
            uint8_t channel = database.read(DB_BLOCK_ENCODERS, dbSection_encoders_midiChannel, i);
            encoderType_t type = static_cast<encoderType_t>(database.read(DB_BLOCK_ENCODERS, dbSection_encoders_mode, i));
            bool validType = true;

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
                if (encoderState == encMoveLeft)
                {
                    if (ccValue[i])
                        ccValue[i]--;
                }
                else
                {
                    if (ccValue[i] < 127)
                        ccValue[i]++;
                }

                encoderValue = ccValue[i];
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