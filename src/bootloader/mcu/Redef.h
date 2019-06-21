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

//redefine SPM_PAGESIZE and BOOT_START_ADDR depending on variant

#ifdef BOARD_A_xu2
#ifdef VARIANT_MEGA
#undef SPM_PAGESIZE
#define SPM_PAGESIZE 256
#undef BOOT_START_ADDR
#define BOOT_START_ADDR 0x3F000
#elif defined(VARIANT_UNO)
#undef SPM_PAGESIZE
#define SPM_PAGESIZE 128
#undef BOOT_START_ADDR
#define BOOT_START_ADDR 0x7800
#endif
#endif