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

// workaround to avoid lots of warnings once the real cmsis_gcc.h gets included due to missing extern "C"

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#if defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6100100)
#include "cmsis_armclang.h"
#elif defined(__GNUC__)
#include "cmsis_gcc.h"
#else
#error Unsupported compiler
#endif

#ifdef __cplusplus
}
#endif