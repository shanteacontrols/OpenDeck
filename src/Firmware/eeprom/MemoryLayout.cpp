#include "Configuration.h"
#include "../hardware/board/Board.h"
#include "../interface/settings/MIDIsettings.h"
#include "../interface/settings/LEDsettings.h"

void Configuration::createMemoryLayout()   {

    /*
        *BLOCK CONTENT DEFINITON*

        section             number of sections inside block
        subsectionType      value formatting of parameters inside block
                                *BIT_PARAMETER   8 parameters per byte
                                *BYTE_PARAMETER  1 parameter per byte
        defaultValue        any value 0-127 or AUTO_INCREMENT (next value gets increased by 1)
        sectionParameters   number of parameters in section
    */

    {
        blocks[CONF_MIDI_BLOCK].sections = MIDI_SECTIONS;

        blocks[CONF_MIDI_BLOCK].subsectionType[midiFeatureSection] = BIT_PARAMETER;
        blocks[CONF_MIDI_BLOCK].defaultValue[midiFeatureSection] = 0;
        blocks[CONF_MIDI_BLOCK].sectionParameters[midiFeatureSection] = MIDI_FEATURES;

        blocks[CONF_MIDI_BLOCK].subsectionType[midiChannelSection] = BYTE_PARAMETER;
        blocks[CONF_MIDI_BLOCK].defaultValue[midiChannelSection] = 0x01;
        blocks[CONF_MIDI_BLOCK].sectionParameters[midiChannelSection] = MIDI_CHANNELS;
    }

    {
        blocks[CONF_BUTTON_BLOCK].sections = BUTTON_SECTIONS;

        blocks[CONF_BUTTON_BLOCK].subsectionType[buttonTypeSection] = BIT_PARAMETER;
        blocks[CONF_BUTTON_BLOCK].defaultValue[buttonTypeSection] = 0;
        blocks[CONF_BUTTON_BLOCK].sectionParameters[buttonTypeSection] = MAX_NUMBER_OF_BUTTONS;

        blocks[CONF_BUTTON_BLOCK].subsectionType[buttonProgramChangeEnabledSection] = BIT_PARAMETER;
        blocks[CONF_BUTTON_BLOCK].defaultValue[buttonProgramChangeEnabledSection] = 0;
        blocks[CONF_BUTTON_BLOCK].sectionParameters[buttonProgramChangeEnabledSection] = MAX_NUMBER_OF_BUTTONS;

        blocks[CONF_BUTTON_BLOCK].subsectionType[buttonMIDIidSection] = BYTE_PARAMETER;
        blocks[CONF_BUTTON_BLOCK].defaultValue[buttonMIDIidSection] = AUTO_INCREMENT;
        blocks[CONF_BUTTON_BLOCK].sectionParameters[buttonMIDIidSection] = MAX_NUMBER_OF_BUTTONS;
    }

    {
        blocks[CONF_ENCODER_BLOCK].sections = ENCODER_SECTIONS;

        blocks[CONF_ENCODER_BLOCK].subsectionType[encoderEnabledSection] = BIT_PARAMETER;
        blocks[CONF_ENCODER_BLOCK].defaultValue[encoderEnabledSection] = 0;
        blocks[CONF_ENCODER_BLOCK].sectionParameters[encoderEnabledSection] = MAX_NUMBER_OF_ENCODERS;

        blocks[CONF_ENCODER_BLOCK].subsectionType[encoderInvertedSection] = BIT_PARAMETER;
        blocks[CONF_ENCODER_BLOCK].defaultValue[encoderInvertedSection] = 0;
        blocks[CONF_ENCODER_BLOCK].sectionParameters[encoderInvertedSection] = MAX_NUMBER_OF_ENCODERS;

        blocks[CONF_ENCODER_BLOCK].subsectionType[encoderEncodingModeSection] = BYTE_PARAMETER;
        blocks[CONF_ENCODER_BLOCK].defaultValue[encoderEncodingModeSection] = enc7Fh01h;
        blocks[CONF_ENCODER_BLOCK].sectionParameters[encoderEncodingModeSection] = MAX_NUMBER_OF_ENCODERS;

        blocks[CONF_ENCODER_BLOCK].subsectionType[encoderMIDIidSection] = BYTE_PARAMETER;
        blocks[CONF_ENCODER_BLOCK].defaultValue[encoderMIDIidSection] = AUTO_INCREMENT;
        blocks[CONF_ENCODER_BLOCK].sectionParameters[encoderMIDIidSection] = MAX_NUMBER_OF_ENCODERS;
    }

    {
        blocks[CONF_ANALOG_BLOCK].sections = ANALOG_SECTIONS;

        blocks[CONF_ANALOG_BLOCK].subsectionType[analogEnabledSection] = BIT_PARAMETER;
        blocks[CONF_ANALOG_BLOCK].defaultValue[analogEnabledSection] = 0;
        blocks[CONF_ANALOG_BLOCK].sectionParameters[analogEnabledSection] = MAX_NUMBER_OF_ANALOG;

        blocks[CONF_ANALOG_BLOCK].subsectionType[analogInvertedSection] = BIT_PARAMETER;
        blocks[CONF_ANALOG_BLOCK].defaultValue[analogInvertedSection] = 0;
        blocks[CONF_ANALOG_BLOCK].sectionParameters[analogInvertedSection] = MAX_NUMBER_OF_ANALOG;

        blocks[CONF_ANALOG_BLOCK].subsectionType[analogTypeSection] = BYTE_PARAMETER;
        blocks[CONF_ANALOG_BLOCK].defaultValue[analogTypeSection] = 0;
        blocks[CONF_ANALOG_BLOCK].sectionParameters[analogTypeSection] = MAX_NUMBER_OF_ANALOG;

        blocks[CONF_ANALOG_BLOCK].subsectionType[analogMIDIidSection] = BYTE_PARAMETER;
        blocks[CONF_ANALOG_BLOCK].defaultValue[analogMIDIidSection] = AUTO_INCREMENT;
        blocks[CONF_ANALOG_BLOCK].sectionParameters[analogMIDIidSection] = MAX_NUMBER_OF_ANALOG;

        blocks[CONF_ANALOG_BLOCK].subsectionType[analogCClowerLimitSection] = BYTE_PARAMETER;
        blocks[CONF_ANALOG_BLOCK].defaultValue[analogCClowerLimitSection] = 0;
        blocks[CONF_ANALOG_BLOCK].sectionParameters[analogCClowerLimitSection] = MAX_NUMBER_OF_ANALOG;

        blocks[CONF_ANALOG_BLOCK].subsectionType[analogCCupperLimitSection] = BYTE_PARAMETER;
        blocks[CONF_ANALOG_BLOCK].defaultValue[analogCCupperLimitSection] = 127;
        blocks[CONF_ANALOG_BLOCK].sectionParameters[analogCCupperLimitSection] = MAX_NUMBER_OF_ANALOG;
    }

    {
        blocks[CONF_LED_BLOCK].sections = LED_SECTIONS;

        blocks[CONF_LED_BLOCK].subsectionType[ledHardwareParameterSection] = BYTE_PARAMETER;
        blocks[CONF_LED_BLOCK].defaultValue[ledHardwareParameterSection] = 0;
        blocks[CONF_LED_BLOCK].sectionParameters[ledHardwareParameterSection] = LED_HARDWARE_PARAMETERS;

        blocks[CONF_LED_BLOCK].subsectionType[ledActivationNoteSection] = BYTE_PARAMETER;
        blocks[CONF_LED_BLOCK].defaultValue[ledActivationNoteSection] = AUTO_INCREMENT;
        blocks[CONF_LED_BLOCK].sectionParameters[ledActivationNoteSection] = MAX_NUMBER_OF_LEDS;

        blocks[CONF_LED_BLOCK].subsectionType[ledStartUpNumberSection] = BYTE_PARAMETER;
        blocks[CONF_LED_BLOCK].defaultValue[ledStartUpNumberSection] = AUTO_INCREMENT;
        blocks[CONF_LED_BLOCK].sectionParameters[ledStartUpNumberSection] = MAX_NUMBER_OF_LEDS;

        blocks[CONF_LED_BLOCK].subsectionType[ledRGBenabledSection] = BIT_PARAMETER;
        blocks[CONF_LED_BLOCK].defaultValue[ledRGBenabledSection] = 0;
        blocks[CONF_LED_BLOCK].sectionParameters[ledRGBenabledSection] = MAX_NUMBER_OF_LEDS;
    }

}
