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

    midiMessageNoteOff              = 0x80, //Note Off
    midiMessageNoteOn               = 0x90, //Note On
    midiMessageControlChange        = 0xB0, //Control Change / Channel Mode
    midiMessageProgramChange        = 0xC0, //Program Change
    midiMessageAfterTouchChannel    = 0xD0, //Channel (monophonic) AfterTouch
    midiMessageAfterTouchPoly       = 0xA0, //Polyphonic AfterTouch
    midiMessagePitchBend            = 0xE0, //Pitch Bend
    midiMessageSystemExclusive      = 0xF0, //System Exclusive
    midiMessageTimeCodeQuarterFrame = 0xF1, //System Common - MIDI Time Code Quarter Frame
    midiMessageSongPosition         = 0xF2, //System Common - Song Position Pointer
    midiMessageSongSelect           = 0xF3, //System Common - Song Select
    midiMessageTuneRequest          = 0xF6, //System Common - Tune Request
    midiMessageClock                = 0xF8, //System Real Time - Timing Clock
    midiMessageStart                = 0xFA, //System Real Time - Start
    midiMessageContinue             = 0xFB, //System Real Time - Continue
    midiMessageStop                 = 0xFC, //System Real Time - Stop
    midiMessageActiveSensing        = 0xFE, //System Real Time - Active Sensing
    midiMessageSystemReset          = 0xFF, //System Real Time - System Reset
    midiMessageInvalidType          = 0x00  //For notifying errors

};

enum MidiFilterMode {

    Off,                //thru disabled (nothing passes through)
    Full,               //fully enabled Thru (every incoming message is sent back)
    SameChannel,        //only the messages on the Input Channel will be sent back
    DifferentChannel    //all the messages but the ones on the Input Channel will be sent back

};

#endif