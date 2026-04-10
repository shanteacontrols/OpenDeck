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

#include <zephyr/devicetree.h>

#if DT_NODE_EXISTS(DT_CHOSEN(opendeck_analog)) && !DT_PROP_OR(DT_CHOSEN(opendeck_analog), disabled, 0)
#if DT_NODE_HAS_PROP(DT_CHOSEN(opendeck_analog), controller_s3_gpios)
#define OPENDECK_ANALOG_COUNT 256
#elif DT_NODE_HAS_PROP(DT_CHOSEN(opendeck_analog), controller_s2_gpios)
#define OPENDECK_ANALOG_COUNT 128
#else
#define OPENDECK_ANALOG_COUNT 64
#endif
#else
#define OPENDECK_ANALOG_COUNT 0
#endif
