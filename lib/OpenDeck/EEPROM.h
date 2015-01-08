/*

OpenDECK library v1.3
File: EEPROM.h
Last revision date: 2014-12-25
Author: Igor Petrovic

*/


#ifndef EEPROM_H_
#define EEPROM_H_

#define EEPROM_M_ID_BYTE_0                  0
#define EEPROM_M_ID_BYTE_1                  1
#define EEPROM_M_ID_BYTE_2                  2

#define EEPROM_HW_CONFIG_START              3

#define EEPROM_BOARD_TYPE                   3
#define EEPROM_HARDWARE_ENABLED             4

#define EEPROM_FEATURES_MIDI                5
#define EEPROM_FEATURES_BUTTONS             6
#define EEPROM_FEATURES_LEDS                7
#define EEPROM_FEATURES_POTS                8

#define EEPROM_MC_START                     9

#define EEPROM_MC_BUTTON_NOTE               9
#define EEPROM_MC_LONG_PRESS_BUTTON_NOTE    10
#define EEPROM_MC_BUTTON_PP                 11
#define EEPROM_MC_POT_CC                    12
#define EEPROM_MC_POT_PP                    13
#define EEPROM_MC_POT_NOTE                  14
#define EEPROM_MC_INPUT                     15

#define EEPROM_BUTTON_TYPE_START            16
#define EEPROM_BUTTON_PP_ENABLED_START      24
#define EEPROM_BUTTON_NOTE_START            32

#define EEPROM_BUTTON_HW_P_START            96
#define EEPROM_BUTTON_HW_P_LONG_PRESS_TIME  96

#define EEPROM_POT_ENABLED_START            97
#define EEPROM_POT_PP_ENABLED_START         105
#define EEPROM_POT_INVERSION_START          113
#define EEPROM_POT_CC_PP_NUMBER_START       121
#define EEPROM_POT_LOWER_LIMIT_START        185
#define EEPROM_POT_UPPER_LIMIT_START        249

#define EEPROM_LED_ACT_NOTE_START           313
#define EEPROM_LED_START_UP_NUMBER_START    377

#define EEPROM_LED_HW_P_START                441
#define EEPROM_LED_HW_P_BLINK_TIME           441
#define EEPROM_LED_HW_P_TOTAL_NUMBER         442
#define EEPROM_LED_HW_P_START_UP_SWITCH_TIME 443
#define EEPROM_LED_HW_P_START_UP_ROUTINE     444


//default controller settings
const uint8_t defConf[] PROGMEM = {

    //manufacturer ID bytes
    0x00, //ID byte 0                       //000
    0x53, //ID byte 1                       //001
    0x43, //ID byte 2                       //002

    //hardware configuration

    //board type
    0x00, //no board                        //003

    //1/enabled, 0/disabled
    //MSB first

    //no function
    //no function
    //no function
    //pots
    //LEDs
    //buttons
    //no function
    0b00001110,                             //004

    //MIDI features

    //no function
    //no function
    //no function
    //no function
    //no function
    //no function
    //no function
    //standard note off
    0b00000000,                             //005

    //button features

    //no function
    //no function
    //no function
    //no function
    //no function
    //no function
    //no function
    //long press
    0b00000001,                             //006

    //LED features

    //no function
    //no function
    //no function
    //no function
    //no function
    //no function
    //LED blinking
    //start-up routine
    0b00000011,                             //007

    //potentiometer features

    //no function
    //no function
    //no function
    //no function
    //no function
    //no function
    //no function
    //potentiometer notes
    0b00000001,                             //008

    //MIDI channels
    0x01, //button note channel             //009
    0x02, //long press note channel         //010
    0x01, //button PP channel               //011
    0x01, //pot CC channel                  //012
    0x01, //pot PP channel                  //013
    0x03, //pot notes channel               //014
    0x01, //MIDI input channel              //015

    //button type
    //0 - press/note on, release/note off (momentary)
    //1 - press/note on, release/nothing, press again/note off (latching)

    //7-0
    0x00,                                   //016
    //15-8
    0x00,
    //23-16
    0x00,
    //31-24
    0x00,
    //39-32
    0x00,
    //47-40
    0x00,
    //55-48
    0x00,
    //63-56
    0x00,

    //enable button PP
    //enabling button PP will disable button notes, and vice versa
    //0 - disabled
    //1 - enabled

    //7-0
    0x00,                                   //024
    //15-8
    0x00,
    //23-16
    0x00,
    //31-24
    0x00,
    //39-32
    0x00,
    //47-40
    0x00,
    //55-48
    0x00,
    //63-56
    0x00,

    //button notes + PP

    0x00,                                   //032
    0x01,
    0x02,
    0x03,
    0x04,
    0x05,
    0x06,
    0x07,
    0x08,
    0x09,
    0x0A,
    0x0B,
    0x0C,
    0x0D,
    0x0E,
    0x0F,
    0x10,
    0x11,
    0x12,
    0x13,
    0x14,
    0x15,
    0x16,
    0x17,
    0x18,
    0x19,
    0x1A,
    0x1B,
    0x1C,
    0x1D,
    0x1E,
    0x1F,
    0x20,
    0x21,
    0x22,
    0x23,
    0x24,
    0x25,
    0x26,
    0x27,
    0x28,
    0x29,
    0x2A,
    0x2B,
    0x2C,
    0x2D,
    0x2E,
    0x2F,
    0x30,
    0x31,
    0x32,
    0x33,
    0x34,
    0x35,
    0x36,
    0x37,
    0x38,
    0x39,
    0x3A,
    0x3B,
    0x3C,
    0x3D,
    0x3E,
    0x3F,

    //button parameters

    //button long press time
    0x05, //x100mS                          //096

    //enable pot (1 enabled, 0 disabled)

    //7-0
    0x00,                                   //097
    //15-8
    0x00,
    //23-16
    0x00,
    //31-24
    0x00,
    //39-32
    0x00,
    //47-40
    0x00,
    //55-48
    0x00,
    //63-56
    0x00,

    //pot PP enabled
    //enabling PP from pots will disable pot CC, and vice versa

    //7-0
    0x00,                                   //105
    //15-8
    0x00,
    //23-16
    0x00,
    //31-24
    0x00,
    //39-32
    0x00,
    //47-40
    0x00,
    //55-48
    0x00,
    //63-56
    0x00,

    //invert potentiometer data (1 invert, 0 don't invert)

    //7-0
    0x00,                                   //113
    //15-8
    0x00,
    //23-16
    0x00,
    //31-24
    0x00,
    //39-32
    0x00,
    //47-40
    0x00,
    //55-48
    0x00,
    //63-56
    0x00,

    //CC/PP potentiometer numbers

    0x00,                                   //121
    0x01,
    0x02,
    0x03,
    0x04,
    0x05,
    0x06,
    0x07,
    0x08,
    0x09,
    0x0A,
    0x0B,
    0x0C,
    0x0D,
    0x0E,
    0x0F,
    0x10,
    0x11,
    0x12,
    0x13,
    0x14,
    0x15,
    0x16,
    0x17,
    0x18,
    0x19,
    0x1A,
    0x1B,
    0x1C,
    0x1D,
    0x1E,
    0x1F,
    0x20,
    0x21,
    0x22,
    0x23,
    0x24,
    0x25,
    0x26,
    0x27,
    0x28,
    0x29,
    0x2A,
    0x2B,
    0x2C,
    0x2D,
    0x2E,
    0x2F,
    0x30,
    0x31,
    0x32,
    0x33,
    0x34,
    0x35,
    0x36,
    0x37,
    0x38,
    0x39,
    0x3A,
    0x3B,
    0x3C,
    0x3D,
    0x3E,
    0x3F,

    //CC/PP lower limits

    0x00,                                   //185
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,

    //CC/PP upper limits

    0x7F,                                   //249
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,

    //LED Activation Notes

    0x7F,                                   //313
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,
    0x7F,

    //LED start-up number

    0x40,                                   //377
    0x40,
    0x40,
    0x40,
    0x40,
    0x40,
    0x40,
    0x40,
    0x40,
    0x40,
    0x40,
    0x40,
    0x40,
    0x40,
    0x40,
    0x40,
    0x40,
    0x40,
    0x40,
    0x40,
    0x40,
    0x40,
    0x40,
    0x40,
    0x40,
    0x40,
    0x40,
    0x40,
    0x40,
    0x40,
    0x40,
    0x40,
    0x40,
    0x40,
    0x40,
    0x40,
    0x40,
    0x40,
    0x40,
    0x40,
    0x40,
    0x40,
    0x40,
    0x40,
    0x40,
    0x40,
    0x40,
    0x40,
    0x40,
    0x40,
    0x40,
    0x40,
    0x40,
    0x40,
    0x40,
    0x40,
    0x40,
    0x40,
    0x40,
    0x40,
    0x40,
    0x40,
    0x40,
    0x40,

    //LED parameters
    0x04, //blink duration time (x100mS)    //441
    0x00, //total number of LEDs            //442
    0x05, //start up LED switch time (x10mS)//443
    0x00, //start-up routine pattern        //444

};

#endif /* EEPROM_H_ */