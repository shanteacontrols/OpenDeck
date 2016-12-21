#pragma once

#include "../../board/variant/Variant.h"

extern volatile bool        blinkEnabled,
                            blinkState;

extern volatile uint8_t     pwmSteps,
                            ledState[MAX_NUMBER_OF_LEDS];

extern volatile uint16_t    ledBlinkTime;

extern volatile int8_t      transitionCounter[MAX_NUMBER_OF_LEDS];
extern volatile uint32_t    blinkTimerCounter;