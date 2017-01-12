#pragma once

typedef enum
{
    ccLimitLow,
    ccLimitHigh
} ccLimitType_t;

typedef enum 
{
    aType_potentiometer,
    aType_fsr,
    aType_button,
    ANALOG_TYPES
} analogType_t;

typedef enum
{
    velocity,
    aftertouch
} pressureType_t;