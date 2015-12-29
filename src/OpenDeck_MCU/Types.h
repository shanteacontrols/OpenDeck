#ifndef TYPES_H_
#define TYPES_H_

enum midiMessageSource   {

    midiSource,
    usbSource

};

enum midiVelocity {

    velocityOn = 127,
    velocityOff = 0

};

enum encoderPosition {

    encMoveLeft = 1,
    encMoveRight = 2,
    encStopped = 0

};

enum buttonType {

    buttonMomentary,
    buttonLatching,
    BUTTON_TYPES

};

enum ledMessageType {

    ledActivationNoteMessage,
    ledStartUpNumberMessage

};

enum ledStates  {

    ledOff = 0,
    ledOn = 0x05,
    ledBlinkOn = 0x16,
    ledBlinkOff = 0x06,
    ledOnRemember = 0x0D,
    ledBlinkRemember = 0x1F

};

enum ledColor {

    colorOff,
    colorWhite,
    colorCyan,
    colorMagenta,
    colorRed,
    colorBlue,
    colorYellow,
    colorGreen,
    colorOnDefault,
    LED_COLORS

};

enum ledType {

    singleLED,
    rgbLED

};

enum encoderType {

    enc7Fh01h = 0,
    enc3Fh41h = 1,
    ENCODING_MODES

};

enum sysExIDcomponentType   {

    buttonComponent = 0,
    encoderComponent = 1,
    analogComponent = 2

};

enum pressureType {

    velocity,
    aftertouch

};

enum analogType {

    potentiometer,
    fsr,
    ldr,
    ANALOG_TYPES

};

enum midiComponentType {

    midiComponentButton,
    midiComponentEncoder,
    midiComponentLED,
    midiComponentAnalog

};

enum midiEventType {

    midiEventNote,
    midiEventCC,
    midiEventProgramChange,
    midiEventAftertouch,
    midiEventHwMIDIpitchBend

};

enum ccLimitType {

    ccLimitLow,
    ccLimitHigh

};


enum midiMessageType {

    midiMessageNoteOff           = 0x80, //Note Off
    midiMessageNoteOn            = 0x90, //Note On
    midiMessageControlChange     = 0xB0, //Control Change / Channel Mode
    midiMessageProgramChange     = 0xC0, //Program Change
    midiMessageAfterTouchChannel = 0xD0, //Channel (monophonic) AfterTouch
    midiMessagePitchBend         = 0xE0, //Pitch Bend
    midiMessageSystemExclusive   = 0xF0, //System Exclusive
    midiMessageInvalidType       = 0x00  //For notifying errors

};

#endif