#ifndef LEDSETTINGS_H_
#define LEDSETTINGS_H_

#define LED_ON_BIT                      0x00
#define LED_BLINK_ON_BIT                0x01
#define LED_ACTIVE_BIT                  0x02
#define LED_REMEMBER_BIT                0x03
#define LED_BLINK_STATE_BIT             0x04

const uint8_t ledOnLookUpTable[] = { 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 0, 0, 0, 0, 0, 0, 0, 255 };

#define NUMBER_OF_TRANSITIONS           64

const uint8_t ledTransitionScale[] = {

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

typedef enum {

    ledsHardwareParameterConf,
    ledsActivationNoteConf,
    ledsStartUpNumberConf,
    ledsRGBcolorConf,
    ledsStateConf,
    LED_SUBTYPES

} sysExMessageSubTypeLEDs;

typedef enum {

    ledsHwParameterTotalLEDnumber,
    ledsHwParameterBlinkTime,
    ledsHwParameterStartUpSwitchTime,
    ledsHwParameterStartUpRoutine,
    ledsHwParameterFadeTime,
    LED_HARDWARE_PARAMETERS

} ledsHardwareParameter;

typedef enum {

    ledStateConstantOff,
    ledStateConstantOn,
    ledStateBlinkOff,
    ledStateBlinkOn,
    LED_STATES

} ledStatesHardwareParameter;

#endif