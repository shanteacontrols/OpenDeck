#pragma once

typedef enum
{
    ledHardwareParameterSection,
    ledActivationNoteSection,
    ledRGBenabledSection,
    ledLocalControlSection,
    LED_SECTIONS
} ledSection;

typedef enum
{
    ledHwParameterBlinkTime,
    ledHwParameterFadeTime,
    ledHwParameterStartUpRoutine,
    LED_HARDWARE_PARAMETERS
} ledHardwareParameter;
