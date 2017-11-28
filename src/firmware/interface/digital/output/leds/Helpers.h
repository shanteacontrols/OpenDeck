#pragma once

#include "Constants.h"

#define LED_ON_MASK     ((0x01 << LED_ACTIVE_BIT) | (0x01 << LED_STATE_BIT))
#define LED_ON(state)   ((state & LED_ON_MASK) == LED_ON_MASK)