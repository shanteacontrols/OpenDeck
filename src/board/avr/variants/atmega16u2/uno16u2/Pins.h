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

#include <avr/io.h>

#define LED_MIDI_IN_DIN_PORT        PORTD
#define LED_MIDI_IN_DIN_PIN         5

#define LED_MIDI_OUT_DIN_PORT       PORTD
#define LED_MIDI_OUT_DIN_PIN        4

#define LED_MIDI_IN_USB_PORT        LED_MIDI_IN_DIN_PORT
#define LED_MIDI_IN_USB_PIN         LED_MIDI_IN_DIN_PIN

#define LED_MIDI_OUT_USB_PORT       LED_MIDI_OUT_DIN_PORT
#define LED_MIDI_OUT_USB_PIN        LED_MIDI_OUT_DIN_PIN