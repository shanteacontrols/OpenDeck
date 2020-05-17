/*

Copyright 2015-2020 Igor Petrovic

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

#include <avr/io.h>

#define DI_1_PORT               PORTD
#define DI_1_PIN                1

#define DI_2_PORT               PORTD
#define DI_2_PIN                0

#define DI_3_PORT               PORTD
#define DI_3_PIN                4

#define DI_4_PORT               PORTC
#define DI_4_PIN                6

#define DI_5_PORT               PORTD
#define DI_5_PIN                7

#define DI_6_PORT               PORTE
#define DI_6_PIN                6


#define DO_1_PORT               PORTB
#define DO_1_PIN                4

#define DO_2_PORT               PORTB
#define DO_2_PIN                5

#define DO_3_PORT               PORTB
#define DO_3_PIN                6

#define DO_4_PORT               PORTB
#define DO_4_PIN                2

#define DO_5_PORT               PORTB
#define DO_5_PIN                3

#define DO_6_PORT               PORTB
#define DO_6_PIN                1


#define AI_1_PORT               PORTF
#define AI_1_PIN                7

#define AI_2_PORT               PORTF
#define AI_2_PIN                6

#define AI_3_PORT               PORTF
#define AI_3_PIN                5

#define AI_4_PORT               PORTF
#define AI_4_PIN                4


#define LED_MIDI_IN_DIN_PORT    PORTD
#define LED_MIDI_IN_DIN_PIN     5

#define LED_MIDI_OUT_DIN_PORT   PORTB
#define LED_MIDI_OUT_DIN_PIN    0

#define LED_MIDI_IN_USB_PORT    LED_MIDI_IN_DIN_PORT
#define LED_MIDI_IN_USB_PIN     LED_MIDI_IN_DIN_PIN

#define LED_MIDI_OUT_USB_PORT   LED_MIDI_OUT_DIN_PORT
#define LED_MIDI_OUT_USB_PIN    LED_MIDI_OUT_DIN_PIN


#define BTLDR_BUTTON_PORT       PORTB
#define BTLDR_BUTTON_PIN        2