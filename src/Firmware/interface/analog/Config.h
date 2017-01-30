/*
    OpenDeck MIDI platform firmware
    Copyright (C) 2015-2017 Igor Petrovic

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

//power of 2
#define NUMBER_OF_SAMPLES               8
//2^3 = 8 (number of samples)
#define SAMPLE_SHIFT                    3
#define MIDI_SHIFT                      3
#define DIGITAL_VALUE_THRESHOLD         1000
//potentiometer must exceed this value before sending new value
#define POTENTIOMETER_CC_STEP           3
#define FSR_MIN_VALUE                   40
#define FSR_MAX_VALUE                   340

#define AFTERTOUCH_MAX_VALUE            600
//ignore aftertouch reading change below this timeout
#define AFTERTOUCH_SEND_TIMEOUT_IGNORE  25
#define AFTERTOUCH_SEND_TIMEOUT_STEP    2
#define AFTERTOUCH_SEND_TIMEOUT         100

#define ENABLE_HYSTERESIS
#define HYSTERESIS_THRESHOLD            950
#define HYSTERESIS_ADDITION             25
#define DISABLE_DIFF_THRESHOLD          250