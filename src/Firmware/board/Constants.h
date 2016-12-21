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
#define LED_CONSTANT_ON_BIT                 0x00
#define LED_BLINK_ON_BIT                    0x01
#define LED_ACTIVE_BIT                      0x02
#define LED_BLINK_STATE_BIT                 0x03

const uint8_t ledOnLookUpTable[] =
{
    0,
    0,
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
    255,
    0
};

const uint8_t ledTransitionScale[] =
{
    0,
    2,
    4,
    6,
    8,
    10,
    12,
    14,
    16,
    18,
    20,
    22,
    24,
    26,
    28,
    30,
    32,
    34,
    36,
    38,
    40,
    42,
    44,
    46,
    48,
    50,
    52,
    54,
    56,
    58,
    60,
    62,
    64,
    68,
    70,
    75,
    80,
    85,
    90,
    95,
    100,
    105,
    110,
    115,
    120,
    125,
    130,
    135,
    140,
    145,
    150,
    155,
    160,
    165,
    170,
    180,
    190,
    200,
    210,
    220,
    230,
    240,
    250,
    255
};