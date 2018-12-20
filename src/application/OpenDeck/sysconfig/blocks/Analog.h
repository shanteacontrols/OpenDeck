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

#include "database/blocks/Analog.h"

typedef enum
{
    sysExSection_analog_enable,
    sysExSection_analog_invert,
    sysExSection_analog_type,
    sysExSection_analog_midiID_LSB,
    sysExSection_analog_midiID_MSB,
    sysExSection_analog_lowerLimit_LSB,
    sysExSection_analog_lowerLimit_MSB,
    sysExSection_analog_upperLimit_LSB,
    sysExSection_analog_upperLimit_MSB,
    sysExSection_analog_midiChannel,
    SYSEX_SECTIONS_ANALOG
} sysExSection_analog_t;

//map sysex sections to sections in db
const uint8_t sysEx2DB_analog[SYSEX_SECTIONS_ANALOG] =
{
    dbSection_analog_enable,
    dbSection_analog_invert,
    dbSection_analog_type,
    dbSection_analog_midiID,
    dbSection_analog_midiID,
    dbSection_analog_lowerLimit,
    dbSection_analog_lowerLimit,
    dbSection_analog_upperLimit,
    dbSection_analog_upperLimit,
    dbSection_analog_midiChannel
};