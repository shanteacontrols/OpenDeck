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

#include "Global.h"
#include "Buttons.h"
#include "Encoders.h"
#include "Analog.h"
#include "LEDs.h"
#include "Display.h"

//define block names
enum sysExBlocks
{
    SYSEX_BLOCK_GLOBAL,    //0
    SYSEX_BLOCK_BUTTONS,   //1
    SYSEX_BLOCK_ENCODERS,  //2
    SYSEX_BLOCK_ANALOG,    //3
    SYSEX_BLOCK_LEDS,      //4
    SYSEX_BLOCK_DISPLAY,   //5
    SYSEX_BLOCKS
};
