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

#include <inttypes.h>

typedef enum
{
    dbSection_display_features,
    dbSection_display_hw,
    DB_SECTIONS_DISPLAY
} dbSection_display_t;

typedef enum
{
    displayHwController,
    displayHwResolution,
    DISPLAY_HW_PARAMETERS
} displayHw_db_t;

typedef enum
{
    displayController_ssd1306,
    DISPLAY_CONTROLLERS
} displayController_t;

typedef enum
{
    displayRes_128x64,
    displayRes_128x32,
    DISPLAY_RESOLUTIONS
} displayResolution_t;

typedef enum
{
    displayFeatureEnable,
    displayFeatureWelcomeMsg,
    displayFeatureVInfoMsg,
    displayFeatureMIDIeventRetention,
    displayFeatureMIDInotesAlternate,
    displayFeatureMIDIeventTime,
    displayFeatureOctaveNormalization,
    DISPLAY_FEATURES
} displayFeature_t;

typedef enum
{
    displayEventIn,
    displayEventOut,
} displayEventType_t;