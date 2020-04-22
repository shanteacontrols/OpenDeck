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

#pragma once

///
/// \brief Size of array used to store all digital input readings.
/// Values are stored in byte array - one bit represents single digital input value.
///
#ifndef NUMBER_OF_BUTTON_COLUMNS
#if ((MAX_NUMBER_OF_BUTTONS % 8) != 0)
#define DIGITAL_IN_ARRAY_SIZE ((MAX_NUMBER_OF_BUTTONS / 8) + 1)
#else
#define DIGITAL_IN_ARRAY_SIZE (MAX_NUMBER_OF_BUTTONS / 8)
#endif
#else
#define DIGITAL_IN_ARRAY_SIZE NUMBER_OF_BUTTON_COLUMNS
#endif

///
/// \brief Size of ring buffer used to store all digital input readings.
/// Once digital input array is full (all inputs are read), index within ring buffer
/// is incremented (if there is space left).
///
#define DIGITAL_IN_BUFFER_SIZE 5

///
/// \brief Time in milliseconds during which MIDI event indicators on board are on when MIDI event happens.
///
#define MIDI_INDICATOR_TIMEOUT 50

///
/// \brief Time in milliseconds for single startup animation cycle on built-in LED indicators.
///
#define LED_INDICATOR_STARTUP_DELAY 150