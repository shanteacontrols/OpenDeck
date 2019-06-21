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

#include "database/blocks/Global.h"

typedef enum
{
    sysExSection_global_midiFeature,
    sysExSection_global_midiMerge,
    sysExSection_global_presets,
    SYSEX_SECTIONS_GLOBAL
} sysExSection_global_t;

//map sysex sections to sections in db
const uint8_t sysEx2DB_midi[SYSEX_SECTIONS_GLOBAL] =
{
    dbSection_global_midiFeatures,
    dbSection_global_midiMerge,
    0 //unused
};