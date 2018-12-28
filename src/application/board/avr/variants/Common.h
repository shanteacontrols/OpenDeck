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
#define ANALOG_STEP_MIN_DIFF_14_BIT     4

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
#define NUMBER_OF_LED_TRANSITIONS       64

namespace Board
{
    namespace detail
    {
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

        #ifdef LED_INDICATORS
        ///
        /// \brief Variables used to control the time MIDI in/out LED indicators on board are active.
        /// When these LEDs need to be turned on, variables are set to value representing time in
        /// milliseconds during which they should be on. ISR decreases variable value by 1 every 1 millsecond.
        /// Once the variables have value 0, specific LED indicator is turned off.
        /// @{

        extern volatile uint8_t midiIn_timeout;
        extern volatile uint8_t midiOut_timeout;

        /// @}
        #endif
    }
}