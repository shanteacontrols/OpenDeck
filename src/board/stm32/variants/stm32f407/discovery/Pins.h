/*

Copyright 2015-2020 Igor Petrovic

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

#include "stm32f4xx_hal.h"

#define DI_1_PORT               GPIOC
#define DI_1_PIN                GPIO_PIN_5

#define DI_2_PORT               GPIOE
#define DI_2_PIN                GPIO_PIN_7

#define DI_3_PORT               GPIOE
#define DI_3_PIN                GPIO_PIN_9

#define DI_4_PORT               GPIOE
#define DI_4_PIN                GPIO_PIN_11

#define DI_5_PORT               GPIOE
#define DI_5_PIN                GPIO_PIN_13

#define DI_6_PORT               GPIOE
#define DI_6_PIN                GPIO_PIN_15

#define DI_7_PORT               GPIOD
#define DI_7_PIN                GPIO_PIN_9

#define DI_8_PORT               GPIOD
#define DI_8_PIN                GPIO_PIN_11

#define DI_9_PORT               GPIOE
#define DI_9_PIN                GPIO_PIN_8

#define DI_10_PORT              GPIOE
#define DI_10_PIN               GPIO_PIN_10

#define DI_11_PORT              GPIOE
#define DI_11_PIN               GPIO_PIN_12

#define DI_12_PORT              GPIOE
#define DI_12_PIN               GPIO_PIN_14

#define DI_13_PORT              GPIOB
#define DI_13_PIN               GPIO_PIN_12

#define DI_14_PORT              GPIOD
#define DI_14_PIN               GPIO_PIN_10

#define DI_15_PORT              GPIOC
#define DI_15_PIN               GPIO_PIN_8

#define DI_16_PORT              GPIOC
#define DI_16_PIN               GPIO_PIN_6

#define DI_17_PORT              GPIOC
#define DI_17_PIN               GPIO_PIN_11

#define DI_18_PORT              GPIOA
#define DI_18_PIN               GPIO_PIN_15


#define DO_1_PORT               GPIOE
#define DO_1_PIN                GPIO_PIN_6

#define DO_2_PORT               GPIOE
#define DO_2_PIN                GPIO_PIN_4

#define DO_3_PORT               GPIOE
#define DO_3_PIN                GPIO_PIN_2

#define DO_4_PORT               GPIOE
#define DO_4_PIN                GPIO_PIN_0

#define DO_5_PORT               GPIOB
#define DO_5_PIN                GPIO_PIN_8

#define DO_6_PORT               GPIOB
#define DO_6_PIN                GPIO_PIN_4

#define DO_7_PORT               GPIOD
#define DO_7_PIN                GPIO_PIN_7

#define DO_8_PORT               GPIOD
#define DO_8_PIN                GPIO_PIN_3

#define DO_9_PORT               GPIOD
#define DO_9_PIN                GPIO_PIN_1

#define DO_10_PORT              GPIOC
#define DO_10_PIN               GPIO_PIN_13

#define DO_11_PORT              GPIOE
#define DO_11_PIN               GPIO_PIN_5

#define DO_12_PORT              GPIOB
#define DO_12_PIN               GPIO_PIN_7

#define DO_13_PORT              GPIOB
#define DO_13_PIN               GPIO_PIN_5

#define DO_14_PORT              GPIOD
#define DO_14_PIN               GPIO_PIN_6

#define DO_15_PORT              GPIOD
#define DO_15_PIN               GPIO_PIN_2

#define DO_16_PORT              GPIOD
#define DO_16_PIN               GPIO_PIN_0


#define AI_1_PORT               GPIOA
#define AI_1_PIN                GPIO_PIN_1

#define AI_2_PORT               GPIOA
#define AI_2_PIN                GPIO_PIN_2

#define AI_3_PORT               GPIOA
#define AI_3_PIN                GPIO_PIN_3

#define AI_4_PORT               GPIOB
#define AI_4_PIN                GPIO_PIN_0

#define AI_5_PORT               GPIOB
#define AI_5_PIN                GPIO_PIN_1

#define AI_6_PORT               GPIOC
#define AI_6_PIN                GPIO_PIN_1

#define AI_7_PORT               GPIOC
#define AI_7_PIN                GPIO_PIN_2

#define AI_8_PORT               GPIOC
#define AI_8_PIN                GPIO_PIN_4


#define LED_MIDI_IN_DIN_PORT    GPIOD
#define LED_MIDI_IN_DIN_PIN     GPIO_PIN_15

#define LED_MIDI_OUT_DIN_PORT   GPIOD
#define LED_MIDI_OUT_DIN_PIN    GPIO_PIN_13

#define LED_MIDI_IN_USB_PORT    GPIOD
#define LED_MIDI_IN_USB_PIN     GPIO_PIN_14

#define LED_MIDI_OUT_USB_PORT   GPIOD
#define LED_MIDI_OUT_USB_PIN    GPIO_PIN_12


#define UART_0_RX_PORT          GPIOB
#define UART_0_RX_PIN           GPIO_PIN_11

#define UART_0_TX_PORT          GPIOD
#define UART_0_TX_PIN           GPIO_PIN_8