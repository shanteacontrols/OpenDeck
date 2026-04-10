/*

Copyright Igor Petrovic

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

#if defined(CONFIG_PROJECT_TARGET_DRIVER_DIGITAL_INPUT_NATIVE)
#include "native/count.h"
#elif defined(CONFIG_PROJECT_TARGET_DRIVER_DIGITAL_INPUT_SHIFT_REGISTER)
#include "shift_register/count.h"
#elif defined(CONFIG_PROJECT_TARGET_DRIVER_DIGITAL_INPUT_MATRIX_NATIVE_ROWS)
#include "matrix_native_rows/count.h"
#elif defined(CONFIG_PROJECT_TARGET_DRIVER_DIGITAL_INPUT_MATRIX_SHIFT_REGISTER_ROWS)
#include "matrix_shift_register_rows/count.h"
#else
#define OPENDECK_BUTTON_COUNT 0
#endif

#define OPENDECK_ENCODER_COUNT (OPENDECK_BUTTON_COUNT / 2)

#define OPENDECK_DIGITAL_INPUT_COUNT OPENDECK_BUTTON_COUNT
