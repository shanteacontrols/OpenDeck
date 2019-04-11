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

typedef enum
{
    dbSection_global_midiFeatures,
    dbSection_global_midiMerge,
    DB_SECTIONS_GLOBAL
} dbSection_global_t;

typedef enum
{
    midiFeatureStandardNoteOff,
    midiFeatureRunningStatus,
    midiFeatureMergeEnabled,
    midiFeatureDinEnabled,
    MIDI_FEATURES
} midiFeature_t;

typedef enum
{
    midiMergeType,
    midiMergeUSBchannel,
    midiMergeDINchannel,
    MIDI_MERGE_OPTIONS
} midiMerge_t;

typedef enum
{
    midiMergeDINtoUSB,
    midiMergeDINtoDIN,
    midiMergeODmaster,
    midiMergeODslave,
    midiMergeODslaveInitial,
    MIDI_MERGE_TYPES
} midiMergeType_t;