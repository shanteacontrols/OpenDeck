#ifndef TYPES_H_
#define TYPES_H_


enum sysExSource   {

    midiSource,
    usbSource

};

enum midiVelocity {

    velocityOn = 127,
    velocityOff = 0

};

enum encoderPosition {

    encMoveLeft = 1,
    encMoveRight = 127,
    encStopped = 0

};

enum buttonType {

    buttonMomentary,
    buttonLatching

};


#endif