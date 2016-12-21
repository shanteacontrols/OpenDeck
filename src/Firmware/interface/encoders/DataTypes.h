#pragma once

typedef enum
{
    enc7Fh01h,
    enc3Fh41h,
    ENCODING_MODES
} encoderType_t;

typedef enum
{
    encStopped,
    encMoveLeft,
    encMoveRight,
} encoderPosition_t;