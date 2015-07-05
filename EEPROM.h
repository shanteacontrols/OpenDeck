/*

OpenDECK library v1.3
File: EEPROM.h
Last revision date: 2014-12-25
Author: Igor Petrovic

*/


#ifndef EEPROM_H_
#define EEPROM_H_

#define EEPROM_M_ID_BYTE_0                          0
#define EEPROM_M_ID_BYTE_1                          1
#define EEPROM_M_ID_BYTE_2                          2

#define EEPROM_MC_START                             3
#define EEPROM_MC_END                               8

#define EEPROM_MC_BUTTON_NOTE                       3
#define EEPROM_MC_PROGRAM_CHANGE                    5
#define EEPROM_MC_CC                                6
#define EEPROM_MC_PITCH_BEND                        7
#define EEPROM_MC_INPUT                             8

#define EEPROM_FEATURES_START                       9
#define EEPROM_FEATURES_END                         13

#define EEPROM_FEATURES_MIDI                        9
#define EEPROM_FEATURES_BUTTONS                     10
#define EEPROM_FEATURES_LEDS                        11
#define EEPROM_FEATURES_POTS                        12
#define EEPROM_FEATURES_ENCODERS                    13

#define EEPROM_BUTTONS_START                        14
#define EEPROM_BUTTONS_END                          98

#define EEPROM_BUTTONS_HW_P_START                   14
#define EEPROM_BUTTONS_HW_P_END                     18

#define EEPROM_BUTTONS_TYPE_START                   19
#define EEPROM_BUTTONS_TYPE_END                     26

#define EEPROM_BUTTONS_PC_ENABLED_START             27
#define EEPROM_BUTTONS_PC_ENABLED_END               34

#define EEPROM_BUTTONS_NOTE_START                   35
#define EEPROM_BUTTONS_NOTE_END                     98

#define EEPROM_ANALOG_START                         99
#define EEPROM_ANALOG_END                           375

#define EEPROM_ANALOG_HW_P_START                    99
#define EEPROM_ANALOG_HW_P_END                      103

#define EEPROM_ANALOG_ENABLED_START                 104
#define EEPROM_ANALOG_ENABLED_END                   111

#define EEPROM_ANALOG_TYPE_START                    112
#define EEPROM_ANALOG_TYPE_END                      175

#define EEPROM_ANALOG_INVERTED_START                176
#define EEPROM_ANALOG_INVERTED_END                  183

#define EEPROM_ANALOG_NUMBER_START                  184
#define EEPROM_ANALOG_NUMBER_END                    247

#define EEPROM_ANALOG_LOWER_LIMIT_START             248
#define EEPROM_ANALOG_LOWER_LIMIT_END               311

#define EEPROM_ANALOG_UPPER_LIMIT_START             312
#define EEPROM_ANALOG_UPPER_LIMIT_END               375

#define EEPROM_ENCODER_START                        376
#define EEPROM_ENCODER_END                          524

#define EEPROM_ENCODERS_HW_P_START                  376
#define EEPROM_ENCODERS_HW_P_END                    380

#define EEPROM_ENCODERS_ENABLED_START               381
#define EEPROM_ENCODERS_ENABLED_END                 388

#define EEPROM_ENCODERS_INVERTED_START              389
#define EEPROM_ENCODERS_INVERTED_END                396

#define EEPROM_ENCODERS_FAST_MODE_START             397
#define EEPROM_ENCODERS_FAST_MODE_END               405

#define EEPROM_ENCODERS_PULSES_PER_STEP_START       406
#define EEPROM_ENCODERS_PULSES_PER_STEP_END         468

#define EEPROM_ENCODERS_ENCODING_MODE_START         469
#define EEPROM_ENCODERS_ENCODING_MODE_END           532

#define EEPROM_ENCODERS_ACCELERATION_START          533
#define EEPROM_ENCODERS_ACCELERATION_END            596

#define EEPROM_ENCODERS_SENSITIVITY_START           597
#define EEPROM_ENCODERS_SENSITIVITY_END             660

#define EEPROM_ENCODERS_NUMBER_START                661
#define EEPROM_ENCODERS_NUMBER_END                  724

#define EEPROM_LEDS_START                           725
#define EEPROM_LEDS_END                             865

#define EEPROM_LEDS_HW_P_START                      725
#define EEPROM_LEDS_HW_P_END                        729

#define EEPROM_LEDS_HW_P_BLINK_TIME                 725
#define EEPROM_LEDS_HW_P_TOTAL_NUMBER               726
#define EEPROM_LEDS_HW_P_START_UP_SWITCH_TIME       727
#define EEPROM_LEDS_HW_P_START_UP_ROUTINE           728

#define EEPROM_LEDS_LOCAL_CONTROL_ENABLED_START     730
#define EEPROM_LEDS_LOCAL_CONTROL_ENABLED_END       737

#define EEPROM_LEDS_ACT_NOTE_START                  738
#define EEPROM_LEDS_ACT_NOTE_END                    801

#define EEPROM_LEDS_START_UP_NUMBER_START           802
#define EEPROM_LEDS_START_UP_NUMBER_END             865


//default controller settings
const uint8_t defConf[] PROGMEM = {

    //manufacturer ID bytes
    0x00, //ID byte 0                       //000
    0x53, //ID byte 1                       //001
    0x43, //ID byte 2                       //002

    //MIDI channels
    0x01, //button note channel             //003
    0x00, //reserved                        //004
    0x01, //program change channel          //005
    0x01, //CC channel                      //006
    0x01, //pitch bend channel              //007
    0x01, //MIDI input channel              //008

    //features
    //MIDI features

    //no function
    //no function
    //no function
    //no function
    //no function
    //no function
    //no function
    //standard note off
    0b00000000,                             //009

    //button features

    //no function
    //no function
    //no function
    //no function
    //no function
    //no function
    //no function
    //no function
    0b00000000,                             //010

    //LED features

    //no function
    //no function
    //no function
    //no function
    //no function
    //no function
    //no function
    //no function
    0b00000000,                             //011

    //potentiometer features

    //no function
    //no function
    //no function
    //no function
    //no function
    //no function
    //no function
    //no function
    0b00000000,                             //012

    //encoder features

    //no function
    //no function
    //no function
    //no function
    //no function
    //no function
    //no function
    //no function
    0b00000000,                             //013

    //buttons
    //button hardware parameters

    0x00, //reserved                        //014
    0x00, //reserved                        //015
    0x00, //reserved                        //016
    0x00, //reserved                        //017
    0x00, //reserved                        //018

    //button type
    //0 - press/note on, release/note off (momentary)
    //1 - press/note on, release/nothing, press again/note off (latching)

    //7-0
    0x00,                                   //019
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

    //enable button program change
    //enabling button program change will disable button notes, and vice versa
    //0 - disabled
    //1 - enabled

    //7-0
    0x00,                                   //027
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

    //button notes/program change

    0x00,                                   //035
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

    //analog hardware parameters
    0x00,  //reserved                       //099
    0x00,  //reserved                       //100
    0x00,  //reserved                       //101
    0x00,  //reserved                       //102
    0x00,  //reserved                       //103

    //enable analog (1 enabled, 0 disabled)

    //7-0
    0x00,                                   //104
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

    //analog connection type
    //0 - potentiometer
    //1 - button
    //2 - FSR
    0x00,                                   //112
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

    //invert analog data (1 invert, 0 don't invert)

    //7-0
    0x00,                                   //176
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

    //CC numbers

    0x00,                                   //184
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

    //CC lower limits

    0x00,                                   //248
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

    //CC upper limits

    0x7F,                                   //312
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

    //encoders
    //encoder hardware parameters
    0x00, //reserved                        //376
    0x00, //reserved                        //377
    0x00, //reserved                        //378
    0x00, //reserved                        //379
    0x00, //reserved                        //380

    //enable encoder (1 enabled, 0 disabled)

    //7-0
    0x00,                                   //381
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

    //invert encoder
    //7-0
    0x00,                                   //389
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

    //fast encoder mode
    //7-0
    0x00,                                   //397
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

    //pulses per step
    0x04,                                   //405
    0x04,
    0x04,
    0x04,
    0x04,
    0x04,
    0x04,
    0x04,
    0x04,
    0x04,
    0x04,
    0x04,
    0x04,
    0x04,
    0x04,
    0x04,
    0x04,
    0x04,
    0x04,
    0x04,
    0x04,
    0x04,
    0x04,
    0x04,
    0x04,
    0x04,
    0x04,
    0x04,
    0x04,
    0x04,
    0x04,
    0x04,
    0x04,
    0x04,
    0x04,
    0x04,
    0x04,
    0x04,
    0x04,
    0x04,
    0x04,
    0x04,
    0x04,
    0x04,
    0x04,
    0x04,
    0x04,
    0x04,
    0x04,
    0x04,
    0x04,
    0x04,
    0x04,
    0x04,
    0x04,
    0x04,
    0x04,
    0x04,
    0x04,
    0x04,
    0x04,
    0x04,
    0x04,
    0x04,

    //encoding mode
    0x00,                                   //469
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

    //encoder acceleration
    0x00,                                   //533
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

    //encoder sensitivity
    0x00,                                   //597
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


    //CC numbers

    0x40,                                   //661
    0x41,
    0x42,
    0x43,
    0x44,
    0x45,
    0x46,
    0x47,
    0x48,
    0x49,
    0x4A,
    0x4B,
    0x4C,
    0x4D,
    0x4E,
    0x4F,
    0x50,
    0x51,
    0x52,
    0x53,
    0x54,
    0x55,
    0x56,
    0x57,
    0x58,
    0x59,
    0x5A,
    0x5B,
    0x5C,
    0x5D,
    0x5E,
    0x5F,
    0x60,
    0x61,
    0x62,
    0x63,
    0x64,
    0x65,
    0x66,
    0x67,
    0x68,
    0x69,
    0x6A,
    0x6B,
    0x6C,
    0x6D,
    0x6E,
    0x6F,
    0x70,
    0x71,
    0x72,
    0x73,
    0x74,
    0x75,
    0x76,
    0x77,
    0x78,
    0x79,
    0x7A,
    0x7B,
    0x7C,
    0x7D,
    0x7E,
    0x7F,


    //LED hardware parameters

    0x04, //blink duration time (x100mS)    //725
    0x00, //total number of LEDs            //726
    0x05, //start up LED switch time (x10mS)//727
    0x00, //start-up routine pattern        //728
    0x00, //reserved                        //729

    //use local LED control instead of MIDI in
    //7-0
    0x00,                                   //730
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

    //LED activation notes/buttons

    0x7F,                                   //738
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

    0x40,                                   //802
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
    0x40

};

#endif /* EEPROM_H_ */