#pragma once

#include "../../eeprom/Database.h"

//define extra sysex section for setting/getting led states
#define ledStateSection                 LED_SECTIONS

#define BLINK_TIME_MIN                  0x00
#define BLINK_TIME_MAX                  0x0F

#define START_UP_SWITCH_TIME_MIN        0x00
#define START_UP_SWITCH_TIME_MAX        0x78

#define FADE_TIME_MIN                   0x00
#define FADE_TIME_MAX                   0x0A

#define NUMBER_OF_START_UP_ANIMATIONS   5