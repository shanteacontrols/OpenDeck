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

#include <inttypes.h>

#ifdef OPENDECK_FW_APP

#define GENERATE_INDICATOR(type)             \
    namespace                                \
    {                                        \
        volatile uint16_t _inTimeout##type;  \
        volatile uint16_t _outTimeout##type; \
    }

#define UPDATE_INDICATOR_TYPE(type)                                           \
    do                                                                        \
    {                                                                         \
        if (_inTimeout##type)                                                 \
        {                                                                     \
            _inTimeout##type--;                                               \
            if (!_inTimeout##type)                                            \
            {                                                                 \
                LED_OFF(INDICATOR_PORT_IN(type), INDICATOR_INDEX_IN(type));   \
            }                                                                 \
        }                                                                     \
        if (_outTimeout##type)                                                \
        {                                                                     \
            _outTimeout##type--;                                              \
            if (!_outTimeout##type)                                           \
            {                                                                 \
                LED_OFF(INDICATOR_PORT_OUT(type), INDICATOR_INDEX_OUT(type)); \
            }                                                                 \
        }                                                                     \
    } while (0)

#define INDICATE_TRAFFIC_TYPE(type, direction)                           \
    do                                                                   \
    {                                                                    \
        switch (direction)                                               \
        {                                                                \
        case board::io::indicators::direction_t::INCOMING:               \
        {                                                                \
            _inTimeout##type = LED_TRAFFIC_INDICATOR_TIMEOUT;            \
            LED_ON(INDICATOR_PORT_IN(type), INDICATOR_INDEX_IN(type));   \
        }                                                                \
        break;                                                           \
        case board::io::indicators::direction_t::OUTGOING:               \
        {                                                                \
            _outTimeout##type = LED_TRAFFIC_INDICATOR_TIMEOUT;           \
            LED_ON(INDICATOR_PORT_OUT(type), INDICATOR_INDEX_OUT(type)); \
        }                                                                \
        break;                                                           \
        }                                                                \
    } while (0)

#endif