#pragma once

#define DIGITAL_BUFFER_SIZE                 2

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

#define NUMBER_OF_LED_TRANSITIONS           64
#define LED_ACTIVE_BIT                      0x00
#define LED_CONSTANT_ON_BIT                 0x01
#define LED_BLINK_ON_BIT                    0x02
#define LED_BLINK_STATE_BIT                 0x03

const uint8_t ledOnLookUpTable[] =
{
    0,
    0,
    0,
    255,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    255
};

const uint8_t ledTransitionScale[] =
{
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    1,
    1,
    1,
    1,
    2,
    2,
    2,
    2,
    3,
    3,
    4,
    4,
    5,
    5,
    6,
    6,
    7,
    8,
    9,
    10,
    11,
    12,
    13,
    14,
    16,
    17,
    19,
    21,
    23,
    25,
    28,
    30,
    33,
    36,
    40,
    44,
    48,
    52,
    57,
    62,
    68,
    74,
    81,
    89,
    97,
    106,
    115,
    126,
    138,
    150,
    164,
    179,
    195,
    213,
    232,
    255
};