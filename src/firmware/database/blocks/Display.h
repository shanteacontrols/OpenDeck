/*
    OpenDeck MIDI platform firmware
    Copyright (C) 2015-2018 Igor Petrovic

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

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