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

#if defined(CONFIG_PROJECT_TARGET_DRIVER_DIGITAL_OUTPUT_NATIVE)
#include "native/native_driver.h"
#elif defined(CONFIG_PROJECT_TARGET_DRIVER_DIGITAL_OUTPUT_SHIFT_REGISTER)
#include "shift_register/shift_register_driver.h"
#elif defined(CONFIG_PROJECT_TARGET_DRIVER_DIGITAL_OUTPUT_MATRIX_NATIVE_ROWS)
#include "matrix_native_rows/matrix_native_rows_driver.h"
#elif defined(CONFIG_PROJECT_TARGET_DRIVER_DIGITAL_OUTPUT_MAX7219)
#include "max7219/max7219_driver.h"
#else
#error "No LED driver selected through DT/Kconfig."
#endif
