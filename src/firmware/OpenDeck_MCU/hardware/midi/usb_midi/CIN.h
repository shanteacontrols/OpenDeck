#ifndef CIN_H_
#define CIN_H_

//define MIDI cable index numbers
//midi10.pdf, page 16

#define CIN_NOTE_ON             0x09
#define CIN_NOTE_OFF            0x08
#define CIN_SYSEX_START         0x04
#define CIN_SYSEX_STOP_1BYTE    0x05
#define CIN_SYSEX_STOP_2BYTE    0x06
#define CIN_SYSEX_STOP_3BYTE    0x07
#define CIN_CONTROL_CHANGE      0x0B
#define CIN_PROGRAM_CHANGE      0x0C
#define CIN_CHANNEL_AFTERTOUCH  0x0D
#define CIN_KEY_AFTERTOUCH      0x0A
#define CIN_PITCH_BEND          0x0E
#define CIN_SYSTEM_COMMON_2BYTE 0x02
#define CIN_SYSTEM_COMMON_3BYTE 0x03


#endif