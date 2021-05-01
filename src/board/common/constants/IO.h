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

#pragma once

/// Size of array used to store all analog readings.
#ifndef NUMBER_OF_MUX
#define ANALOG_IN_BUFFER_SIZE MAX_NUMBER_OF_ANALOG
#else
#define ANALOG_IN_BUFFER_SIZE (NUMBER_OF_MUX_INPUTS * NUMBER_OF_MUX)
#endif

/// Time in milliseconds during which MIDI event indicators on board are on when MIDI event happens.
#define MIDI_INDICATOR_TIMEOUT 50

/// Time in milliseconds for single startup animation cycle on built-in LED indicators.
#define LED_INDICATOR_STARTUP_DELAY 150