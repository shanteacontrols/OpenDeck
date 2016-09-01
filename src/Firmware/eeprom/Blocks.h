#ifndef BLOCKS_H_
#define BLOCKS_H_

//define block names
enum eepromBlocks {

    CONF_BLOCK_MIDI,    //0
    CONF_BLOCK_BUTTON,  //1
    CONF_BLOCK_ENCODER, //2
    CONF_BLOCK_ANALOG,  //3
    CONF_BLOCK_LED,     //4
    CONF_BLOCKS

};

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
    ledRGBenabledSection,
    LED_SECTIONS

} ledSection;

#endif