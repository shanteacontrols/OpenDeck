/*

Copyright 2015-2022 Igor Petrovic

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

/// Time in milliseconds during which MIDI event indicators on board are on when MIDI event happens.
constexpr inline uint32_t LED_INDICATOR_TIMEOUT = 50;

/// Time in milliseconds for single startup animation cycle on built-in LED indicators.
constexpr inline uint32_t LED_INDICATOR_STARTUP_DELAY = 150;