#pragma once

/*
    Encoder data formatting, uint16_t variable type
    0      1      2      3
    0000 | 0000 | 0000 | 0000

    0 - encoder direction (0/1 - left/right)
    1 - encoderMoving (0/1/2 - stopped/left/right)
    2 - counted pulses (default value is 8 to avoid issues with negative values)
    3 - temp encoder state (2 readings of 2 encoder pairs)
*/

#define ENCODER_CLEAR_TEMP_STATE_MASK       0xFFF0
#define ENCODER_CLEAR_PULSES_MASK           0xFF0F
#define ENCODER_CLEAR_MOVING_STATUS_MASK    0xF0FF
#define ENCODER_DIRECTION_BIT               15
#define ENCODER_DEFAULT_PULSE_COUNT_STATE   8
#define PULSES_PER_STEP                     4

const int8_t encoderLookUpTable[] =
{
    0,
    1,
    -1,
    2,
    -1,
    0,
    -2,
    1,
    1,
    -2,
    0,
    -1,
    2,
    -1,
    1,
    0
};