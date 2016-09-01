#ifndef SPECIALREQUESTS_H_
#define SPECIALREQUESTS_H_

typedef enum {

    HANDSHAKE_REQUEST,          //00
    BYTES_PER_VALUE_REQUEST,    //01
    PARAMS_PER_MESSAGE_REQUEST, //02
    SPECIAL_PARAMETERS

} sysEx_specialRequestID;

#endif