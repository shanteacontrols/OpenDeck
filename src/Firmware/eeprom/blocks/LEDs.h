#pragma once

typedef enum
{
    ledHardwareParameterSection,
    ledActivationNoteSection,
    ledStartUpNumberSection,
    ledRGBenabledSection,
    ledLocalControlEnabled,
    LED_SECTIONS
} ledSection;

typedef enum
{
    ledHwParameterTotalLEDnumber,
    ledHwParameterBlinkTime,
    ledHwParameterStartUpSwitchTime,
    ledHwParameterStartUpRoutine,
    ledHwParameterFadeTime,
    LED_HARDWARE_PARAMETERS
} ledHardwareParameter;
