#ifndef SECTIONS_H_
#define SECTIONS_H_

typedef enum {

    midiFeatureSection,
    midiChannelSection,
    MIDI_SECTIONS

} midiSection;

typedef enum {

    buttonTypeSection,
    buttonProgramChangeEnabledSection,
    buttonMIDIidSection,
    BUTTON_SECTIONS

} buttonSection;

typedef enum {

    encoderEnabledSection,
    encoderInvertedSection,
    encoderEncodingModeSection,
    encoderMIDIidSection,
    ENCODER_SECTIONS

} encoderSection;

typedef enum {

    analogEnabledSection,
    analogTypeSection,
    analogInvertedSection,
    analogMIDIidSection,
    analogCClowerLimitSection,
    analogCCupperLimitSection,
    ANALOG_SECTIONS

} analogSection;

typedef enum {

    ledHardwareParameterSection,
    ledActivationNoteSection,
    ledStartUpNumberSection,
    ledRGBcolorSection,
    LED_SECTIONS

} ledSection;

#endif