#ifndef SYSEX_STATUS_H_
#define SYSEX_STATUS_H_

typedef enum {

    REQUEST,                //0x00
    ACK,                    //0x01
    CUSTOM,                 //0x02
    ERROR_STATUS,           //0x03
    ERROR_HANDSHAKE,        //0x04
    ERROR_WISH,             //0x05
    ERROR_AMOUNT,           //0x06
    ERROR_BLOCK,            //0x07
    ERROR_SECTION,          //0x08
    ERROR_PART,             //0x09
    ERROR_PARAMETER,        //0x0A
    ERROR_NEW_PARAMETER,    //0x0B
    ERROR_MESSAGE_LENGTH,   //0x0C
    ERROR_WRITE             //0x0D

} sysExStatus_t;

#endif