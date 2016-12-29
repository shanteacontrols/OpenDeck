#pragma once

typedef enum
{
    ccLimitLow,
    ccLimitHigh
} ccLimitType_t;

typedef enum 
{
    potentiometer,
    fsr,
    ldr,
    ANALOG_TYPES
} analogType_t;

typedef enum
{
    velocity,
    aftertouch
} pressureType_t;