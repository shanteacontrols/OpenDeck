/*
             LUFA Library
     Copyright (C) Dean Camera, 2017.

  dean [at] fourwalledcubicle [dot] com
           www.lufa-lib.org
*/

/*
  Copyright 2017  Dean Camera (dean [at] fourwalledcubicle [dot] com)

  Permission to use, copy, modify, distribute, and sell this
  software and its documentation for any purpose is hereby granted
  without fee, provided that the above copyright notice appear in
  all copies and that both that the copyright notice and this
  permission notice and warranty disclaimer appear in supporting
  documentation, and that the name of the author not be used in
  advertising or publicity pertaining to distribution of the
  software without specific, written prior permission.

  The author disclaims all warranties with regard to this
  software, including all implied warranties of merchantability
  and fitness.  In no event shall the author be liable for any
  special, indirect or consequential damages or any damages
  whatsoever resulting from loss of use, data or profits, whether
  in an action of contract, negligence or other tortious action,
  arising out of or in connection with the use or performance of
  this software.
*/

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

#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/boot.h>
#include <avr/power.h>
#include <avr/interrupt.h>
#include <stdbool.h>

#ifdef USB_SUPPORTED
#include "Descriptors.h"
#include <LUFA/Drivers/USB/USB.h>
#include <LUFA/Platform/Platform.h>
#else
#include <LUFA/Common/Common.h>
#endif
#include "core/src/general/Misc.h"
#include "Config.h"

#if !defined(__OPTIMIZE_SIZE__)
#error This bootloader requires that it be optimized for size, not speed, to fit into the target device. Change optimization settings and try again.
#endif

#ifdef USB_SUPPORTED
void EVENT_USB_Device_ControlRequest();
void EVENT_USB_Device_ConfigurationChanged();
#else
void EVENT_UART_Device_ControlRequest();
#endif