/*
    OpenDeck MIDI platform firmware
    Copyright (C) 2015, 2016 Igor Petrovic

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef TYPES_H_
#define TYPES_H_

enum midiInterfaceType_t   {

    dinInterface,
    usbInterface

};

enum midiVelocity_t {

    velocityOn = 127,
    velocityOff = 0

};

enum encoderPosition_t {

    encMoveLeft = 1,
    encMoveRight = 2,
    encStopped = 0

};

enum buttonType_t {

    buttonMomentary,
    buttonLatching,
    BUTTON_TYPES

};

enum ledMessageType_t {

    ledActivationNoteMessage,
    ledStartUpNumberMessage

};

enum ledColor_t {

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

enum ledType_t {

    singleLED,
    rgbLED

};

enum encoderType_t {

    enc7Fh01h = 0,
    enc3Fh41h = 1,
    ENCODING_MODES

};

enum pressureType_t {

    velocity,
    aftertouch

};

enum analogType_t {

    potentiometer,
    fsr,
    ldr,
    ANALOG_TYPES

};

enum midiComponentType_t {

    midiComponentButton,
    midiComponentEncoder,
    midiComponentLED,
    midiComponentAnalog

};

enum midiEventType_t {

    midiEventNote,
    midiEventCC,
    midiEventProgramChange,
    midiEventAftertouch,
    midiEventHwMIDIpitchBend

};

enum ccLimitType_t {

    ccLimitLow,
    ccLimitHigh

};

enum midiMessageType_t {

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

enum usbMIDIsystemCin_t {

    //normally, usb midi cin (cable index number) is just midiMessageType shifted left by four bytes
    //system common/exclusive messages have a bit convulted pattern so they're grouped in different enum
    sysCommon1byteCin = 0x50,
    sysCommon2byteCin = 0x20,
    sysCommon3byteCin = 0x30,
    sysExStartCin = 0x40,
    sysExStop1byteCin = sysCommon1byteCin,
    sysExStop2byteCin = 0x60,
    sysExStop3byteCin = 0x70

};

enum midiFilterMode_t {

    Off,                //thru disabled (nothing passes through)
    Full,               //fully enabled Thru (every incoming message is sent back)
    SameChannel,        //only the messages on the Input Channel will be sent back
    DifferentChannel    //all the messages but the ones on the Input Channel will be sent back

};

typedef enum {

    noteOffType_noteOnZeroVel,
    noteOffType_standardNoteOff

} noteOffType_t;

typedef enum {

    swVersion_major,
    swVersion_minor,
    swVersion_revision,
    swVersion_development

} swVersion_t;

typedef enum {

    factoryReset_wipeRestore, //clear eeprom, restore defaults
    factoryReset_restore, //update eeprom with defaults
    factoryReset_partial //partially restore defaults

} factoryResetType_t;

#endif