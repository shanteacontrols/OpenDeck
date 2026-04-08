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

#ifdef PROJECT_TARGET_LEDS_INT_INVERT
#define LED_ON(port, pin)    CORE_MCU_IO_SET_LOW(port, pin)
#define LED_OFF(port, pin)   CORE_MCU_IO_SET_HIGH(port, pin)
#define IS_LED_ON(port, pin) !CORE_MCU_IO_READ(port, pin)
#else
#define LED_ON(port, pin)    CORE_MCU_IO_SET_HIGH(port, pin)
#define LED_OFF(port, pin)   CORE_MCU_IO_SET_LOW(port, pin)
#define IS_LED_ON(port, pin) CORE_MCU_IO_READ(port, pin)
#endif

#define INDICATOR_PORT_IN(type)   PIN_PORT_LED_IND_IN_##type
#define INDICATOR_INDEX_IN(type)  PIN_INDEX_LED_IND_IN_##type
#define INDICATOR_PORT_OUT(type)  PIN_PORT_LED_IND_OUT_##type
#define INDICATOR_INDEX_OUT(type) PIN_INDEX_LED_IND_OUT_##type

#define INIT_INDICATOR_TYPE(type)                                     \
    do                                                                \
    {                                                                 \
        CORE_MCU_IO_INIT(INDICATOR_PORT_IN(type),                     \
                         INDICATOR_INDEX_IN(type),                    \
                         core::mcu::io::pinMode_t::OUTPUT_PP,         \
                         core::mcu::io::pullMode_t::NONE);            \
        CORE_MCU_IO_INIT(INDICATOR_PORT_OUT(type),                    \
                         INDICATOR_INDEX_OUT(type),                   \
                         core::mcu::io::pinMode_t::OUTPUT_PP,         \
                         core::mcu::io::pullMode_t::NONE);            \
        LED_OFF(INDICATOR_PORT_IN(type), INDICATOR_INDEX_IN(type));   \
        LED_OFF(INDICATOR_PORT_OUT(type), INDICATOR_INDEX_OUT(type)); \
    } while (0)

#define ALL_INDICATOR_TYPE_ON(type)                                  \
    do                                                               \
    {                                                                \
        LED_ON(INDICATOR_PORT_IN(type), INDICATOR_INDEX_IN(type));   \
        LED_ON(INDICATOR_PORT_OUT(type), INDICATOR_INDEX_OUT(type)); \
    } while (0)

#define ALL_INDICATOR_TYPE_OFF(type)                                  \
    do                                                                \
    {                                                                 \
        LED_OFF(INDICATOR_PORT_IN(type), INDICATOR_INDEX_IN(type));   \
        LED_OFF(INDICATOR_PORT_OUT(type), INDICATOR_INDEX_OUT(type)); \
    } while (0)

#define IN_INDICATOR_TYPE_ON(type)                                 \
    do                                                             \
    {                                                              \
        LED_ON(INDICATOR_PORT_IN(type), INDICATOR_INDEX_IN(type)); \
    } while (0)

#define IN_INDICATOR_TYPE_OFF(type)                                 \
    do                                                              \
    {                                                               \
        LED_OFF(INDICATOR_PORT_IN(type), INDICATOR_INDEX_IN(type)); \
    } while (0)

#define OUT_INDICATOR_TYPE_ON(type)                                  \
    do                                                               \
    {                                                                \
        LED_ON(INDICATOR_PORT_OUT(type), INDICATOR_INDEX_OUT(type)); \
    } while (0)

#define OUT_INDICATOR_TYPE_OFF(type)                                  \
    do                                                                \
    {                                                                 \
        LED_OFF(INDICATOR_PORT_OUT(type), INDICATOR_INDEX_OUT(type)); \
    } while (0)
