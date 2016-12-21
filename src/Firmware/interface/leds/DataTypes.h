#pragma once

#include <inttypes.h>

typedef struct
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
} rgbValue_t;

typedef enum
{
    rgb_R,
    rgb_G,
    rgb_B
} rgbIndex_t;