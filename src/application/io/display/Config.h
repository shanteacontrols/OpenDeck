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
/// \brief Size of buffer used to build text string on display in bytes.
///
#define LCD_STRING_BUFFER_SIZE 40

///
/// \brief Length of temporary (message) text on display in milliseconds.
///
#define LCD_MESSAGE_DURATION 1500

///
/// \brief Time in milliseconds after text on display is being refreshed.
///
#define LCD_REFRESH_TIME 10

///
/// \brief Time in milliseconds after which scrolling text moves on display.
///
#define LCD_SCROLL_TIME 1000

///
/// \brief Maximum amount of characters displayed in single LCD row.
/// Real width is determined later based on display type.
///
#define LCD_WIDTH_MAX 32

///
/// \brief Maximum number of LCD rows.
/// Real height is determined later based on display type.
///
#define LCD_HEIGHT_MAX 4

#define COLUMN_START_MIDI_IN_MESSAGE  4
#define COLUMN_START_MIDI_OUT_MESSAGE 5

#define ROW_START_MIDI_IN_MESSAGE  0
#define ROW_START_MIDI_OUT_MESSAGE 2