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
/// \brief Various commands and IDs used to communicate with SDW display.
/// @{

#define START_BYTE       0xAA
#define HANDSHAKE_BYTE1  0x00
#define HANDSHAKE_BYTE2  0x0A
#define END_CODE1        0xCC
#define END_CODE2        0x33
#define END_CODE3        0xC3
#define END_CODE4        0x3C
#define END_CODES        4
#define PICTURE_DISPLAY  0x70
#define PICTURE_CUT      0x71
#define CUT_PICTURE_ID   0x02
#define BUTTON_INDEX_1   0x02
#define BUTTON_INDEX_2   0x03
#define COMMAND_ID_INDEX 0x01
#define BUTTON_ON_ID     0x79
#define BUTTON_OFF_ID    0x78

/// @}