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

#pragma once

///
/// \brief Constant used to debounce button readings.
///
#define BUTTON_DEBOUNCE_COMPARE         0b11110000

///
/// brief Total number of analog components.
///
#ifdef BOARD_A_PRO_MICRO
#define MAX_NUMBER_OF_ANALOG            4
#else
#define MAX_NUMBER_OF_ANALOG            6
#endif

///
/// \brief Maximum number of buttons.
///
#ifdef BOARD_A_PRO_MICRO
#define MAX_NUMBER_OF_BUTTONS           6
#else
#define MAX_NUMBER_OF_BUTTONS           8
#endif

///
/// \brief Maximum number of LEDs.
///
#define MAX_NUMBER_OF_LEDS              6

///
/// \brief Maximum number of RGB LEDs.
/// One RGB LED requires three standard LED connections.
///
#define MAX_NUMBER_OF_RGB_LEDS          (MAX_NUMBER_OF_LEDS/3)

///
/// \brief Maximum number of encoders.
/// Total number of encoders is total number of buttons divided by two.
///
#define MAX_NUMBER_OF_ENCODERS          (MAX_NUMBER_OF_BUTTONS/2)

///
/// \brief If touchscreen isn't supported, set MAX_TOUCHSCREEN_BUTTONS to zero.
///
#define MAX_TOUCHSCREEN_BUTTONS         0