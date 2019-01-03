/*

Copyright 2015-2019 Igor Petrovic

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

#pragma once

#define ENCODER_VALUE_LEFT_7FH01H   127
#define ENCODER_VALUE_RIGHT_7FH01H  1

#define ENCODER_VALUE_LEFT_3FH41H   63
#define ENCODER_VALUE_RIGHT_3FH41H  65

///
/// \brief Time in milliseconds after which debounce mode is reset if encoder isn't moving.
///
#define DEBOUNCE_RESET_TIME     50

///
/// Number of times movement in the same direction must be registered in order
/// for debouncer to become active. Once the debouncer is active, all further changes
/// in the movement will be ignored, that is, all movements will be registered in the
/// direction which was repeated. This state is reset until the encoder is either stopped
/// or if same amount of movements are registered in the opposite direction.
///
#define ENCODER_DEBOUNCE_COUNT  4

///
/// \brief Used to achieve linear encoder acceleration on fast movement.
/// Every time fast movement is detected, amount of steps is increased by this value.
/// Used only in CC/Pitch bend/NRPN modes. In Pitch bend/NRPN modes, this value is multiplied
/// by 4 due to a larger value range.
///
#define ENCODER_SPEED_CHANGE    3

///
/// \brief Maximum value by which MIDI value is increased during acceleration.
///
#define ENCODER_MAX_SPEED       100

///
/// \brief Time threshold in milliseconds between two encoder steps used to detect fast movement.
///
#define SPEED_TIMEOUT           140

///
/// \brief Array used for easier access to current encoder MIDI value in 7Fh01h and 3Fh41h modes.
/// Matched with encoderType_t and encoderPosition_t
///
const uint8_t       encValue[2][3] =
{
    {
        0,
        ENCODER_VALUE_LEFT_7FH01H,
        ENCODER_VALUE_RIGHT_7FH01H
    },

    {
        0,
        ENCODER_VALUE_LEFT_3FH41H,
        ENCODER_VALUE_RIGHT_3FH41H
    }
};