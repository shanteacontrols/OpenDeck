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

#include <inttypes.h>

///
/// \brief Defines how many analog samples from the same input will be thrown away before storing the read value.
///
#define ADC_IGNORED_SAMPLES_COUNT 5

///
/// \brief Total number of states between fully off and fully on for LEDs.
///
#define NUMBER_OF_LED_TRANSITIONS 64