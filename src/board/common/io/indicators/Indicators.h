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

#include "Common.h"
#include "App.h"
#include "Boot.h"

#ifdef USB_INDICATORS_SUPPORTED
GENERATE_INDICATOR(USB)
#define INIT_USB_INDICATOR()            INIT_INDICATOR_TYPE(USB)
#define UPDATE_USB_INDICATOR()          UPDATE_INDICATOR_TYPE(USB)
#define INDICATE_USB_TRAFFIC(direction) INDICATE_TRAFFIC_TYPE(USB, direction)
#define INDICATOR_USB_ON()              INDICATOR_TYPE_ON(USB)
#define INDICATOR_USB_OFF()             INDICATOR_TYPE_OFF(USB)
#else
#define INIT_USB_INDICATOR()
#define UPDATE_USB_INDICATOR()
#define INDICATE_USB_TRAFFIC(direction)
#define INDICATOR_USB_ON()
#define INDICATOR_USB_OFF()
#endif

#ifdef UART_INDICATORS_SUPPORTED
GENERATE_INDICATOR(UART)
#define INIT_UART_INDICATOR()            INIT_INDICATOR_TYPE(UART)
#define UPDATE_UART_INDICATOR()          UPDATE_INDICATOR_TYPE(UART)
#define INDICATE_UART_TRAFFIC(direction) INDICATE_TRAFFIC_TYPE(UART, direction)
#define INDICATOR_UART_ON()              INDICATOR_TYPE_ON(UART)
#define INDICATOR_UART_OFF()             INDICATOR_TYPE_OFF(UART)
#else
#define INIT_UART_INDICATOR()
#define UPDATE_UART_INDICATOR()
#define INDICATE_UART_TRAFFIC(direction)
#define INDICATOR_UART_ON()
#define INDICATOR_UART_OFF()
#endif

#ifdef BLE_INDICATORS_SUPPORTED
GENERATE_INDICATOR(BLE)
#define INIT_BLE_INDICATOR()            INIT_INDICATOR_TYPE(BLE)
#define UPDATE_BLE_INDICATOR()          UPDATE_INDICATOR_TYPE(BLE)
#define INDICATE_BLE_TRAFFIC(direction) INDICATE_TRAFFIC_TYPE(BLE, direction)
#define INDICATOR_BLE_ON()              INDICATOR_TYPE_ON(BLE)
#define INDICATOR_BLE_OFF()             INDICATOR_TYPE_OFF(BLE)
#else
#define INIT_BLE_INDICATOR()
#define UPDATE_BLE_INDICATOR()
#define INDICATE_BLE_TRAFFIC(direction)
#define INDICATOR_BLE_ON()
#define INDICATOR_BLE_OFF()
#endif

#define ALL_INDICATORS_ON()  \
    do                       \
    {                        \
        INDICATOR_USB_ON();  \
        INDICATOR_UART_ON(); \
        INDICATOR_BLE_ON();  \
    } while (0)

#define ALL_INDICATORS_OFF()  \
    do                        \
    {                         \
        INDICATOR_USB_OFF();  \
        INDICATOR_UART_OFF(); \
        INDICATOR_BLE_OFF();  \
    } while (0)

#define INIT_ALL_INDICATORS()  \
    do                         \
    {                          \
        INIT_USB_INDICATOR();  \
        INIT_UART_INDICATOR(); \
        INIT_BLE_INDICATOR();  \
    } while (0)
