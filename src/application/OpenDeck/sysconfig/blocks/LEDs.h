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

#include "database/blocks/LEDs.h"

typedef enum
{
    sysExSection_leds_testColor,
    sysExSection_leds_testBlink,
    sysExSection_leds_global,
    sysExSection_leds_activationID,
    sysExSection_leds_rgbEnable,
    sysExSection_leds_controlType,
    sysExSection_leds_activationValue,
    sysExSection_leds_midiChannel,
    SYSEX_SECTIONS_LEDS
} sysExSection_leds_t;

//map sysex sections to sections in db
const uint8_t sysEx2DB_leds[SYSEX_SECTIONS_LEDS] = {
    0,
    0,
    dbSection_leds_global,
    dbSection_leds_activationID,
    dbSection_leds_rgbEnable,
    dbSection_leds_controlType,
    dbSection_leds_activationValue,
    dbSection_leds_midiChannel,
};