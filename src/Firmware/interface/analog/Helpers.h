#pragma once

#include "Config.h"

#define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))
#define RAW_ADC_2_MIDI(value)   (value >> MIDI_SHIFT)
#define ADC_AVG_VALUE(value)    (value >> SAMPLE_SHIFT)