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

#include <inttypes.h>

///
/// \brief Value above which buton connected to analog input is considered pressed.
///
#define ADC_DIGITAL_VALUE_THRESHOLD_ON  1000

///
/// \brief Value below which button connected to analog input is considered released.
///
#define ADC_DIGITAL_VALUE_THRESHOLD_OFF 600

///
/// \brief Minimum difference between two raw ADC readings to consider that value has been changed.
/// Used when calculating 7-bit MIDI value.
///
#define ANALOG_STEP_MIN_DIFF_7_BIT      6

///
/// \brief Minimum difference between two raw ADC readings to consider that value has been changed.
/// Used when calculating 14-bit MIDI value.
///
#define ANALOG_STEP_MIN_DIFF_14_BIT     1

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
/// \brief Defines how many analog samples from the same input will be thrown away before storing the read value.
///
#define ADC_IGNORED_SAMPLES_COUNT       3

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
#define NUMBER_OF_LED_TRANSITIONS       64
