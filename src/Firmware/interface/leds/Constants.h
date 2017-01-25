#pragma once

#include "../../database/Database.h"

//define extra sysex sections for setting/getting led states
#define ledColorSection                 LED_SECTIONS
#define ledBlinkSection                 LED_SECTIONS+1

#define BLINK_TIME_MIN                  0x02
#define BLINK_TIME_MAX                  0x0F

#define FADE_TIME_MIN                   0x00
#define FADE_TIME_MAX                   0x0A