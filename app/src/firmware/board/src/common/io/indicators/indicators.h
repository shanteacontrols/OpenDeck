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

#include "common.h"
#include "app.h"
#include "boot.h"

#ifdef PROJECT_TARGET_SUPPORT_USB_INDICATORS
GENERATE_INDICATOR(USB)
#define INIT_USB_INDICATOR()            INIT_INDICATOR_TYPE(USB)
#define UPDATE_USB_INDICATOR()          UPDATE_INDICATOR_TYPE(USB)
#define INDICATE_USB_TRAFFIC(direction) INDICATE_TRAFFIC_TYPE(USB, direction)
#define ALL_INDICATOR_USB_ON()          ALL_INDICATOR_TYPE_ON(USB)
#define ALL_INDICATOR_USB_OFF()         ALL_INDICATOR_TYPE_OFF(USB)
#define IN_INDICATOR_USB_ON()           IN_INDICATOR_TYPE_ON(USB)
#define IN_INDICATOR_USB_OFF()          IN_INDICATOR_TYPE_OFF(USB)
#define OUT_INDICATOR_USB_ON()          OUT_INDICATOR_TYPE_ON(USB)
#define OUT_INDICATOR_USB_OFF()         OUT_INDICATOR_TYPE_OFF(USB)
#else
#define INIT_USB_INDICATOR()
#define UPDATE_USB_INDICATOR()
#define INDICATE_USB_TRAFFIC(direction)
#define ALL_INDICATOR_USB_ON()
#define ALL_INDICATOR_USB_OFF()
#define IN_INDICATOR_USB_ON()
#define IN_INDICATOR_USB_OFF()
#define OUT_INDICATOR_USB_ON()
#define OUT_INDICATOR_USB_OFF()
#endif

#ifdef PROJECT_TARGET_SUPPORT_UART_INDICATORS
GENERATE_INDICATOR(UART)
#define INIT_UART_INDICATOR()            INIT_INDICATOR_TYPE(UART)
#define UPDATE_UART_INDICATOR()          UPDATE_INDICATOR_TYPE(UART)
#define INDICATE_UART_TRAFFIC(direction) INDICATE_TRAFFIC_TYPE(UART, direction)
#define ALL_INDICATOR_UART_ON()          ALL_INDICATOR_TYPE_ON(UART)
#define ALL_INDICATOR_UART_OFF()         ALL_INDICATOR_TYPE_OFF(UART)
#define IN_INDICATOR_UART_ON()           IN_INDICATOR_TYPE_ON(UART)
#define IN_INDICATOR_UART_OFF()          IN_INDICATOR_TYPE_OFF(UART)
#define OUT_INDICATOR_UART_ON()          OUT_INDICATOR_TYPE_ON(UART)
#define OUT_INDICATOR_UART_OFF()         OUT_INDICATOR_TYPE_OFF(UART)
#else
#define INIT_UART_INDICATOR()
#define UPDATE_UART_INDICATOR()
#define INDICATE_UART_TRAFFIC(direction)
#define ALL_INDICATOR_UART_ON()
#define ALL_INDICATOR_UART_OFF()
#define IN_INDICATOR_UART_ON()
#define IN_INDICATOR_UART_OFF()
#define OUT_INDICATOR_UART_ON()
#define OUT_INDICATOR_UART_OFF()
#endif

#ifdef PROJECT_TARGET_SUPPORT_BLE_INDICATORS
GENERATE_INDICATOR(BLE)
#define INIT_BLE_INDICATOR()            INIT_INDICATOR_TYPE(BLE)
#define UPDATE_BLE_INDICATOR()          UPDATE_INDICATOR_TYPE(BLE)
#define INDICATE_BLE_TRAFFIC(direction) INDICATE_TRAFFIC_TYPE(BLE, direction)
#define ALL_INDICATOR_BLE_ON()          ALL_INDICATOR_TYPE_ON(BLE)
#define ALL_INDICATOR_BLE_OFF()         ALL_INDICATOR_TYPE_OFF(BLE)
#define IN_INDICATOR_BLE_ON()           IN_INDICATOR_TYPE_ON(BLE)
#define IN_INDICATOR_BLE_OFF()          IN_INDICATOR_TYPE_OFF(BLE)
#define OUT_INDICATOR_BLE_ON()          OUT_INDICATOR_TYPE_ON(BLE)
#define OUT_INDICATOR_BLE_OFF()         OUT_INDICATOR_TYPE_OFF(BLE)
#else
#define INIT_BLE_INDICATOR()
#define UPDATE_BLE_INDICATOR()
#define INDICATE_BLE_TRAFFIC(direction)
#define ALL_INDICATOR_BLE_ON()
#define ALL_INDICATOR_BLE_OFF()
#define IN_INDICATOR_BLE_ON()
#define IN_INDICATOR_BLE_OFF()
#define OUT_INDICATOR_BLE_ON()
#define OUT_INDICATOR_BLE_OFF()
#endif

#define ALL_INDICATORS_ON()      \
    do                           \
    {                            \
        ALL_INDICATOR_USB_ON();  \
        ALL_INDICATOR_UART_ON(); \
        ALL_INDICATOR_BLE_ON();  \
    } while (0)

#define IN_INDICATORS_ON()      \
    do                          \
    {                           \
        IN_INDICATOR_USB_ON();  \
        IN_INDICATOR_UART_ON(); \
        IN_INDICATOR_BLE_ON();  \
    } while (0)

#define OUT_INDICATORS_ON()      \
    do                           \
    {                            \
        OUT_INDICATOR_USB_ON();  \
        OUT_INDICATOR_UART_ON(); \
        OUT_INDICATOR_BLE_ON();  \
    } while (0)

#define ALL_INDICATORS_OFF()      \
    do                            \
    {                             \
        ALL_INDICATOR_USB_OFF();  \
        ALL_INDICATOR_UART_OFF(); \
        ALL_INDICATOR_BLE_OFF();  \
    } while (0)

#define IN_INDICATORS_OFF()      \
    do                           \
    {                            \
        IN_INDICATOR_USB_OFF();  \
        IN_INDICATOR_UART_OFF(); \
        IN_INDICATOR_BLE_OFF();  \
    } while (0)

#define OUT_INDICATORS_OFF()      \
    do                            \
    {                             \
        OUT_INDICATOR_USB_OFF();  \
        OUT_INDICATOR_UART_OFF(); \
        OUT_INDICATOR_BLE_OFF();  \
    } while (0)

#define INIT_ALL_INDICATORS()  \
    do                         \
    {                          \
        INIT_USB_INDICATOR();  \
        INIT_UART_INDICATOR(); \
        INIT_BLE_INDICATOR();  \
    } while (0)
