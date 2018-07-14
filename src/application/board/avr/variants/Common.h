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

///
/// \brief Value above which digital input connected to analog input is considered pressed.
///
#define DIGITAL_VALUE_THRESHOLD         1000

///
/// \brief Minimum difference between two raw ADC readings to consider that value has been changed.
///
#define ANALOG_STEP_MIN_DIFF            6

///
/// \brief Minimum raw ADC reading for FSR sensors.
///
#define FSR_MIN_VALUE                   40

///
/// \brief Maximum raw ADC reading for FSR sensors.
///
#define FSR_MAX_VALUE                   340

///
/// \brief Maxmimum raw ADC reading for aftertouch on FSR sensors.
///
#define AFTERTOUCH_MAX_VALUE            600

///
/// \brief Minimum raw ADC value.
///
#define ADC_MIN_VALUE                   0

///
/// \brief Maxmimum raw ADC value.
///
#define ADC_MAX_VALUE                   1023

///
/// \brief Total number of raw ADC readings taken for single analog component.
/// Must be power of 2 value because bit shifting is used to calculate average value.
///
#define NUMBER_OF_ANALOG_SAMPLES        1

///
/// \brief Number of bits to shift to get average ADC value.
/// Must correspond with NUMBER_OF_ANALOG_SAMPLES constant.
///
#define ANALOG_SAMPLE_SHIFT             0

///
/// \brief Raw ADC reading above which analog component is considered to be in higher hysteresis region.
///
#define HYSTERESIS_THRESHOLD_HIGH       970

///
/// \brief Raw ADC reading below which analog component is considered to be in lower hysteresis region.
///
#define HYSTERESIS_THRESHOLD_LOW        50

///
/// \brief Value which is added to raw ADC reading if analog component is in higher hysteresis region.
///
#define HYSTERESIS_ADDITION             20

///
/// \brief Value which is subtracted from raw ADC reading if analog component is in lower hysteresis region.
///
#define HYSTERESIS_SUBTRACTION          15

///
/// \brief Location at which reboot type is written in EEPROM when initiating software reset.
/// See Reboot.h
///
#define REBOOT_VALUE_EEPROM_LOCATION    (EEPROM_SIZE - 1)

///
/// \brief Location at which compiled binary CRC is written in EEPROM.
/// CRC takes two bytes.
///
#define SW_CRC_LOCATION_EEPROM          (EEPROM_SIZE - 3)

///
/// \brief Total number of states between fully off and fully on for LEDs.
///
#define NUMBER_OF_LED_TRANSITIONS           64

///
/// \brief Main ISR vector name common for all AVR boards.
///
#define CORE_ISR                            TIMER0_COMPA_vect

///
/// \brief ADC ISR vector name common for all AVR boards.
///
#define ADC_ISR                             ADC_vect

///
/// \brief Array holding values needed to achieve more natural LED transition between states.
///
const uint8_t ledTransitionScale[NUMBER_OF_LED_TRANSITIONS] =
{
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    1,
    1,
    1,
    1,
    2,
    2,
    2,
    2,
    3,
    3,
    4,
    4,
    5,
    5,
    6,
    6,
    7,
    8,
    9,
    10,
    11,
    12,
    13,
    14,
    16,
    17,
    19,
    21,
    23,
    25,
    28,
    30,
    33,
    36,
    40,
    44,
    48,
    52,
    57,
    62,
    68,
    74,
    81,
    89,
    97,
    106,
    115,
    126,
    138,
    150,
    164,
    179,
    195,
    213,
    255
};
