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

#include "descriptors/types/Helpers.h"

#ifdef __AVR__
#include <avr/pgmspace.h>
#include "LUFA/Drivers/USB/USB.h"

#define UNICODE_STRING(string) L##string
#else
#ifdef FW_CDC
#include "descriptors/types/CDCDescriptors.h"
#else
#include "descriptors/types/AudioDescriptors.h"
#include "descriptors/types/MIDIDescriptors.h"
#endif

#define UNICODE_STRING(string) u##string
#endif