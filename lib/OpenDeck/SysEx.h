/*

OpenDECK library v0.99
File: SysEx.h
Last revision date: 2014-09-15
Author: Igor Petrovic

*/


#ifndef SYSEX_H_
#define SYSEX_H_

//sysex

//manufacturer ID bytes
#define SYS_EX_M_ID_0                           0x00
#define SYS_EX_M_ID_1                           0x53
#define SYS_EX_M_ID_2                           0x43

//message wish
#define SYS_EX_WISH_GET                         0x00
#define SYS_EX_WISH_SET                         0x01
#define SYS_EX_WISH_RESTORE                     0x02

//requested data amount
#define SYS_EX_AMOUNT_SINGLE                    0x00
#define SYS_EX_AMOUNT_ALL                       0x01

//message types
#define SYS_EX_MT_MIDI_CHANNEL                  0x4D
#define SYS_EX_MT_HW_PARAMETER                  0x54
#define SYS_EX_MT_FREE_PINS                     0x46
#define SYS_EX_MT_SW_FEATURE                    0x53
#define SYS_EX_MT_HW_FEATURE                    0x48
#define SYS_EX_MT_BUTTON                        0x42
#define SYS_EX_MT_POT                           0x50
#define SYS_EX_MT_ENC                           0x45
#define SYS_EX_MT_LED                           0x4C
#define SYS_EX_MT_ALL                           0x52

//message subtypes

//buttons
#define SYS_EX_MST_BUTTON_TYPE                  0x00
#define SYS_EX_MST_BUTTON_NOTE                  0x01

//pots
#define SYS_EX_MST_POT_ENABLED                  0x00
#define SYS_EX_MST_POT_INVERTED                 0x01
#define SYS_EX_MST_POT_CC_NUMBER                0x02

//LEDs
#define SYS_EX_MST_LED_ACT_NOTE                 0x00
#define SYS_EX_MST_LED_START_UP_NUMBER          0x01
#define SYS_EX_MST_LED_STATE                    0x02

////////////

//parameters

//MIDI channels
#define SYS_EX_MC_BUTTON_NOTE                   0x00
#define SYS_EX_MC_LONG_PRESS_BUTTON_NOTE        0x01
#define SYS_EX_MC_POT_CC                        0x02
#define SYS_EX_MC_ENC_CC                        0x03
#define SYS_EX_MC_INPUT                         0x04

//hardware parameters
#define SYS_EX_HW_P_BOARD_TYPE                  0x00
#define SYS_EX_HW_P_LONG_PRESS_TIME             0x01
#define SYS_EX_HW_P_BLINK_TIME                  0x02
#define SYS_EX_HW_P_TOTAL_LED_NUMBER            0x03
#define SYS_EX_HW_P_START_UP_SWITCH_TIME        0x04
#define SYS_EX_HW_P_START_UP_ROUTINE            0x05

//free pins
#define SYS_EX_FREE_PIN_A                       0x00
#define SYS_EX_FREE_PIN_B                       0x01
#define SYS_EX_FREE_PIN_C                       0x02
#define SYS_EX_FREE_PIN_D                       0x03

//software features
#define SYS_EX_SW_F_RUNNING_STATUS              0x00
#define SYS_EX_SW_F_STANDARD_NOTE_OFF           0x01
#define SYS_EX_SW_F_ENC_NOTES                   0x02
#define SYS_EX_SW_F_POT_NOTES                   0x03
#define SYS_EX_SW_F_LONG_PRESS                  0x04
#define SYS_EX_SW_F_LED_BLINK                   0x05
#define SYS_EX_SW_F_START_UP_ROUTINE            0x06

//hardware features
#define SYS_EX_HW_F_BUTTONS                     0x00
#define SYS_EX_HW_F_POTS                        0x01
#define SYS_EX_HW_F_ENC                         0x02
#define SYS_EX_HW_F_LEDS                        0x03

////////////

#define SYS_EX_ENABLE                           0x01
#define SYS_EX_DISABLE                          0x00

//board types
#define SYS_EX_BOARD_TYPE_OPEN_DECK_1           0x01
#define SYS_EX_BOARD_TYPE_TANNIN                0x02

//button types
#define SYS_EX_BUTTON_TYPE_MOMENTARY            0x00
#define SYS_EX_BUTTON_TYPE_LATCHING             0x01


//LED state control
#define SYS_EX_LED_STATE_C_OFF                  0x00
#define SYS_EX_LED_STATE_C_ON                   0x01
#define SYS_EX_LED_STATE_B_OFF                  0x02
#define SYS_EX_LED_STATE_B_ON                   0x03

//LED blink/constant state determination
#define SYS_EX_LED_VELOCITY_C_ON                0x3E
#define SYS_EX_LED_VELOCITY_C_OFF               0x00
#define SYS_EX_LED_VELOCITY_B_ON                0x7F
#define SYS_EX_LED_VELOCITY_B_OFF               0x3F

//allowed states of free pins
#define SYS_EX_FREE_PIN_STATE_DISABLED          0x00
#define SYS_EX_FREE_PIN_STATE_B_ROW             0x01
#define SYS_EX_FREE_PIN_STATE_L_ROW             0x02

//order of bytes in sysex message (message start)
#define SYS_EX_MS_M_ID_0                        0x01
#define SYS_EX_MS_M_ID_1                        0x02
#define SYS_EX_MS_M_ID_2                        0x03
#define SYS_EX_MS_WISH                          0x04
#define SYS_EX_MS_AMOUNT                        0x05
#define SYS_EX_MS_MT                            0x06
#define SYS_EX_MS_MST                           0x07
#define SYS_EX_MS_PARAMETER_ID                  0x08
#define SYS_EX_MS_NEW_PARAMETER_ID_SINGLE       0x09
#define SYS_EX_MS_NEW_PARAMETER_ID_ALL          0x08

//message length
#define SYS_EX_ML_REQ_HANDSHAKE                 0x05
#define SYS_EX_ML_REQ_DATA                      0x09
#define SYS_EX_ML_RES_BASIC                     0x08

//ACK/error start codes
#define SYS_EX_ACK                              0x41
#define SYS_EX_ERROR                            0x46

//errors
#define SYS_EX_ERROR_HANDSHAKE                  0x00
#define SYS_EX_ERROR_WISH                       0x01
#define SYS_EX_ERROR_AMOUNT                     0x02
#define SYS_EX_ERROR_MT                         0x03
#define SYS_EX_ERROR_MST                        0x04
#define SYS_EX_ERROR_PARAMETER                  0x05
#define SYS_EX_ERROR_NEW_PARAMETER              0x06
#define SYS_EX_ERROR_MESSAGE_LENGTH             0x07
#define SYS_EX_ERROR_NOT_SUPPORTED              0x08
#define SYS_EX_ERROR_EEPROM                     0x09

#endif /* SYSEX_H_ */