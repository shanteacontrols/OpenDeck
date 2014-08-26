/*

OpenDECK library v1.93
File: SysEx.h
Last revision date: 2014-08-26
Author: Igor Petrovic

*/ 


#ifndef SYSEX_H_
#define SYSEX_H_

//sysex

#define SYS_EX_MANUFACTURER_ID_0				0x00
#define SYS_EX_MANUFACTURER_ID_1				0x53
#define SYS_EX_MANUFACTURER_ID_2				0x43

#define SYS_EX_MIDI_CHANNEL_START				0x4D

#define SYS_EX_HW_PARAMETER_START				0x54
#define SYS_EX_SW_FEATURE_START					0x53
#define SYS_EX_HW_FEATURE_START					0x48

#define SYS_EX_BUTTON_START						0x42
#define SYS_EX_POT_START						0x50
#define SYS_EX_ENC_START						0x45
#define SYS_EX_LED_START						0x4C

#define SYS_EX_GET								0x00
#define SYS_EX_SET								0x01
#define SYS_EX_ENABLE							0x01
#define SYS_EX_DISABLE							0x00

#define SYS_EX_GET_SET_SINGLE					0x00
#define SYS_EX_GET_SET_ALL						0x01

#define SYS_EX_GET_SET_BUTTON_TYPE				0x00
#define SYS_EX_GET_SET_BUTTON_NOTE				0x01

#define SYS_EX_GET_SET_POT_ENABLED				0x00
#define SYS_EX_GET_SET_POT_INVERTED				0x01
#define SYS_EX_GET_SET_POT_CC_NUMBER			0x02

#define SYS_EX_GET_SET_LED_ID					0x00

#define SYS_EX_MC_BUTTON_NOTE					0x00
#define SYS_EX_MC_LONG_PRESS_BUTTON_NOTE		0x01
#define SYS_EX_MC_POT_CC						0x02
#define SYS_EX_MC_ENC_CC						0x03
#define SYS_EX_MC_INPUT							0x04

#define SYS_EX_HW_P_LONG_PRESS_TIME				0x00
#define SYS_EX_HW_P_BLINK_TIME					0x01
#define SYS_EX_HW_P_START_UP_SWITCH_TIME		0x02

#define SYS_EX_SW_F_RUNNING_STATUS				0x00
#define SYS_EX_SW_F_STANDARD_NOTE_OFF			0x01
#define SYS_EX_SW_F_ENC_NOTES					0x02
#define SYS_EX_SW_F_POT_NOTES					0x03
#define SYS_EX_SW_F_LONG_PRESS					0x04
#define SYS_EX_SW_F_LED_BLINK					0x05
#define SYS_EX_SW_F_START_UP_ROUTINE			0x06

#define SYS_EX_HW_F_BUTTONS						0x00
#define SYS_EX_HW_F_POTS						0x01
#define SYS_EX_HW_F_ENC							0x02
#define SYS_EX_HW_F_LEDS						0x03

#define SYS_EX_MIN_MESSAGE_LENGHT				0x09
#define SYS_EX_ACK								0x41
#define SYS_EX_ERROR							0x46

#endif /* SYSEX_H_ */