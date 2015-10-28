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

    encMoveLeft = -1,
    encMoveRight = 1,
    encStopped = 0

};

enum buttonType {

    buttonMomentary,
    buttonLatching,
    buttonTypeEnd

};

enum ledStates  {

    ledOff = 0,
    ledOn = 0x05,
    ledBlink = 0x16,
    ledOnRemember = 0x0D,
    ledBlinkRemember = 0x1F

};

enum encoderType {

    enc7Fh01h = 0,
    enc3Fh41h = 1

};

enum sysExIDcomponentType   {

    button = 0,
    encoder = 1,
    analog = 2

};

enum pressureType {

    velocity,
    aftertouch

};

enum analogType {

    potentiometer = 0,
    fsr1 = 1,
    fsr2 = 2,
    fsr3 = 3,
    ldr = 4,
    analogTypeEnd

};


#endif