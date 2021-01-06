/*

Copyright 2015-2021 Igor Petrovic

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

/// Helper macros used for easier control of internal (on-board) and external LEDs.

#ifdef LED_INT_INVERT
#define INT_LED_ON(port, pin)  CORE_IO_SET_LOW(port, pin)
#define INT_LED_OFF(port, pin) CORE_IO_SET_HIGH(port, pin)
#else
#define INT_LED_ON(port, pin)  CORE_IO_SET_HIGH(port, pin)
#define INT_LED_OFF(port, pin) CORE_IO_SET_LOW(port, pin)
#endif

#ifdef LED_EXT_INVERT
#define EXT_LED_ON(port, pin)  CORE_IO_SET_LOW(port, pin)
#define EXT_LED_OFF(port, pin) CORE_IO_SET_HIGH(port, pin)
#else
#define EXT_LED_ON(port, pin)  CORE_IO_SET_HIGH(port, pin)
#define EXT_LED_OFF(port, pin) CORE_IO_SET_LOW(port, pin)
#endif