#pragma once

//power of 2
#define NUMBER_OF_SAMPLES               8
//2^3 = 8 (number of samples)
#define SAMPLE_SHIFT                    3
#define MIDI_SHIFT                      3
#define DIGITAL_VALUE_THRESHOLD         1000
//potentiometer must exceed this value before sending new value
#define POTENTIOMETER_CC_STEP           3
#define FSR_MIN_VALUE                   40
#define FSR_MAX_VALUE                   340

#define AFTERTOUCH_MAX_VALUE            600
//ignore aftertouch reading change below this timeout
#define AFTERTOUCH_SEND_TIMEOUT_IGNORE  25
#define AFTERTOUCH_SEND_TIMEOUT_STEP    2
#define AFTERTOUCH_SEND_TIMEOUT         100