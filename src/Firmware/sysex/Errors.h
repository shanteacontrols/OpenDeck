#ifndef ERRORS_H_
#define ERRORS_H_

typedef enum {

    ERROR_HANDSHAKE,        //0
    ERROR_WISH,             //1
    ERROR_AMOUNT,           //2
    ERROR_BLOCK,            //3
    ERROR_SECTION,          //4
    ERROR_PARAMETER,        //5
    ERROR_NEW_PARAMETER,    //6
    ERROR_MESSAGE_LENGTH,   //7
    ERROR_EEPROM,           //8

} sysExError;


#endif