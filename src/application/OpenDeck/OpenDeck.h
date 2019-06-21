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

#if !defined(BOARD_A_xu2)
#include "database/Database.h"
#include "interface/analog/Analog.h"
#ifdef LEDS_SUPPORTED
#include "interface/digital/output/leds/LEDs.h"
#endif
#ifdef DISPLAY_SUPPORTED
#include "interface/display/Display.h"
#endif
#ifdef TOUCHSCREEN_SUPPORTED
#include "interface/display/touch/Touchscreen.h"
#endif
#include "sysconfig/SysConfig.h"
#endif
#include "midi/src/MIDI.h"

class OpenDeck
{
    public:
    OpenDeck() {}

    void init();
    void update();
    void checkMIDI();
    void checkComponents();
};