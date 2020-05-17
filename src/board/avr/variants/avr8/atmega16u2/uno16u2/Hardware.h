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

#include "../MCU.h"

///
/// \brief Holds current version of hardware.
/// Can be overriden during build process to compile
/// the firmware for different hardware revision of the board.
/// @{

#ifndef HARDWARE_VERSION_MAJOR
#define HARDWARE_VERSION_MAJOR  1
#endif

#ifndef HARDWARE_VERSION_MINOR
#define HARDWARE_VERSION_MINOR  0
#endif

/// @}

///
/// \brief Indicates that the board supports USB MIDI.
///
#define USB_MIDI_SUPPORTED

///
/// \brief Defines total number of available UART interfaces on board.
///
#define UART_INTERFACES         1

///
/// \brief Defines UART channel used for communication with main MCU.
///
#define UART_USB_LINK_CHANNEL   0

///
/// \brief Use integrated LED indicators.
///
#define LED_INDICATORS

///
/// \brief Use inverted logic when controlling integrated LEDs (high/off, low/on).
///
#define LED_INT_INVERT

///
/// \brief Used to specify that this is USB link MCU used only as link between PC and main MCU.
///
#define USB_LINK_MCU

#undef BTLDR_FLASH_PAGE_SIZE
#define BTLDR_FLASH_PAGE_SIZE 128