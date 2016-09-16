#include "Configuration.h"
#include "../interface/settings/MIDIsettings.h"
#include "../interface/settings/LEDsettings.h"

void Configuration::createMemoryLayout()   {

    {
        blocks[CONF_BLOCK_MIDI].sections = MIDI_SECTIONS;

        blocks[CONF_BLOCK_MIDI].sectionParameterType[midiFeatureSection] = BIT_PARAMETER;
        blocks[CONF_BLOCK_MIDI].defaultValue[midiFeatureSection] = 0;
        blocks[CONF_BLOCK_MIDI].sectionParameters[midiFeatureSection] = MIDI_FEATURES;

        blocks[CONF_BLOCK_MIDI].sectionParameterType[midiChannelSection] = BYTE_PARAMETER;
        blocks[CONF_BLOCK_MIDI].defaultValue[midiChannelSection] = 0x01;
        blocks[CONF_BLOCK_MIDI].sectionParameters[midiChannelSection] = MIDI_CHANNELS;
    }

    {
        blocks[CONF_BLOCK_BUTTON].sections = BUTTON_SECTIONS;

        blocks[CONF_BLOCK_BUTTON].sectionParameterType[buttonTypeSection] = BIT_PARAMETER;
        blocks[CONF_BLOCK_BUTTON].defaultValue[buttonTypeSection] = 0;
        blocks[CONF_BLOCK_BUTTON].sectionParameters[buttonTypeSection] = MAX_NUMBER_OF_BUTTONS+MAX_NUMBER_OF_ANALOG;

        blocks[CONF_BLOCK_BUTTON].sectionParameterType[buttonProgramChangeEnabledSection] = BIT_PARAMETER;
        blocks[CONF_BLOCK_BUTTON].defaultValue[buttonProgramChangeEnabledSection] = 0;
        blocks[CONF_BLOCK_BUTTON].sectionParameters[buttonProgramChangeEnabledSection] = MAX_NUMBER_OF_BUTTONS+MAX_NUMBER_OF_ANALOG;

        blocks[CONF_BLOCK_BUTTON].sectionParameterType[buttonMIDIidSection] = BYTE_PARAMETER;
        blocks[CONF_BLOCK_BUTTON].defaultValue[buttonMIDIidSection] = AUTO_INCREMENT;
        blocks[CONF_BLOCK_BUTTON].sectionParameters[buttonMIDIidSection] = MAX_NUMBER_OF_BUTTONS+MAX_NUMBER_OF_ANALOG;
    }

    {
        blocks[CONF_BLOCK_ENCODER].sections = ENCODER_SECTIONS;

        blocks[CONF_BLOCK_ENCODER].sectionParameterType[encoderEnabledSection] = BIT_PARAMETER;
        blocks[CONF_BLOCK_ENCODER].defaultValue[encoderEnabledSection] = 0;
        blocks[CONF_BLOCK_ENCODER].sectionParameters[encoderEnabledSection] = MAX_NUMBER_OF_ENCODERS;

        blocks[CONF_BLOCK_ENCODER].sectionParameterType[encoderInvertedSection] = BIT_PARAMETER;
        blocks[CONF_BLOCK_ENCODER].defaultValue[encoderInvertedSection] = 0;
        blocks[CONF_BLOCK_ENCODER].sectionParameters[encoderInvertedSection] = MAX_NUMBER_OF_ENCODERS;

        blocks[CONF_BLOCK_ENCODER].sectionParameterType[encoderEncodingModeSection] = BYTE_PARAMETER;
        blocks[CONF_BLOCK_ENCODER].defaultValue[encoderEncodingModeSection] = enc7Fh01h;
        blocks[CONF_BLOCK_ENCODER].sectionParameters[encoderEncodingModeSection] = MAX_NUMBER_OF_ENCODERS;

        blocks[CONF_BLOCK_ENCODER].sectionParameterType[encoderMIDIidSection] = BYTE_PARAMETER;
        blocks[CONF_BLOCK_ENCODER].defaultValue[encoderMIDIidSection] = AUTO_INCREMENT;
        blocks[CONF_BLOCK_ENCODER].sectionParameters[encoderMIDIidSection] = MAX_NUMBER_OF_ENCODERS;
    }

    {
        blocks[CONF_BLOCK_ANALOG].sections = ANALOG_SECTIONS;

        blocks[CONF_BLOCK_ANALOG].sectionParameterType[analogEnabledSection] = BIT_PARAMETER;
        blocks[CONF_BLOCK_ANALOG].defaultValue[analogEnabledSection] = 0;
        blocks[CONF_BLOCK_ANALOG].sectionParameters[analogEnabledSection] = MAX_NUMBER_OF_ANALOG;

        blocks[CONF_BLOCK_ANALOG].sectionParameterType[analogInvertedSection] = BIT_PARAMETER;
        blocks[CONF_BLOCK_ANALOG].defaultValue[analogInvertedSection] = 0;
        blocks[CONF_BLOCK_ANALOG].sectionParameters[analogInvertedSection] = MAX_NUMBER_OF_ANALOG;

        blocks[CONF_BLOCK_ANALOG].sectionParameterType[analogTypeSection] = BYTE_PARAMETER;
        blocks[CONF_BLOCK_ANALOG].defaultValue[analogTypeSection] = 0;
        blocks[CONF_BLOCK_ANALOG].sectionParameters[analogTypeSection] = MAX_NUMBER_OF_ANALOG;

        blocks[CONF_BLOCK_ANALOG].sectionParameterType[analogMIDIidSection] = BYTE_PARAMETER;
        blocks[CONF_BLOCK_ANALOG].defaultValue[analogMIDIidSection] = AUTO_INCREMENT;
        blocks[CONF_BLOCK_ANALOG].sectionParameters[analogMIDIidSection] = MAX_NUMBER_OF_ANALOG;

        blocks[CONF_BLOCK_ANALOG].sectionParameterType[analogCClowerLimitSection] = BYTE_PARAMETER;
        blocks[CONF_BLOCK_ANALOG].defaultValue[analogCClowerLimitSection] = 0;
        blocks[CONF_BLOCK_ANALOG].sectionParameters[analogCClowerLimitSection] = MAX_NUMBER_OF_ANALOG;

        blocks[CONF_BLOCK_ANALOG].sectionParameterType[analogCCupperLimitSection] = BYTE_PARAMETER;
        blocks[CONF_BLOCK_ANALOG].defaultValue[analogCCupperLimitSection] = 127;
        blocks[CONF_BLOCK_ANALOG].sectionParameters[analogCCupperLimitSection] = MAX_NUMBER_OF_ANALOG;

        blocks[CONF_BLOCK_ANALOG].sectionParameterType[analogDigitalEnabledSection] = BIT_PARAMETER;
        blocks[CONF_BLOCK_ANALOG].defaultValue[analogDigitalEnabledSection] = 0;
        blocks[CONF_BLOCK_ANALOG].sectionParameters[analogDigitalEnabledSection] = MAX_NUMBER_OF_ANALOG;
    }

    {
        blocks[CONF_BLOCK_LED].sections = LED_SECTIONS;

        blocks[CONF_BLOCK_LED].sectionParameterType[ledHardwareParameterSection] = BYTE_PARAMETER;
        blocks[CONF_BLOCK_LED].defaultValue[ledHardwareParameterSection] = 0;
        blocks[CONF_BLOCK_LED].sectionParameters[ledHardwareParameterSection] = LED_HARDWARE_PARAMETERS;

        blocks[CONF_BLOCK_LED].sectionParameterType[ledActivationNoteSection] = BYTE_PARAMETER;
        blocks[CONF_BLOCK_LED].defaultValue[ledActivationNoteSection] = AUTO_INCREMENT;
        blocks[CONF_BLOCK_LED].sectionParameters[ledActivationNoteSection] = MAX_NUMBER_OF_LEDS;

        blocks[CONF_BLOCK_LED].sectionParameterType[ledStartUpNumberSection] = BYTE_PARAMETER;
        blocks[CONF_BLOCK_LED].defaultValue[ledStartUpNumberSection] = AUTO_INCREMENT;
        blocks[CONF_BLOCK_LED].sectionParameters[ledStartUpNumberSection] = MAX_NUMBER_OF_LEDS;

        blocks[CONF_BLOCK_LED].sectionParameterType[ledRGBenabledSection] = BIT_PARAMETER;
        blocks[CONF_BLOCK_LED].defaultValue[ledRGBenabledSection] = 0;
        blocks[CONF_BLOCK_LED].sectionParameters[ledRGBenabledSection] = MAX_NUMBER_OF_LEDS;

        blocks[CONF_BLOCK_LED].sectionParameterType[ledLocalControlEnabled] = BIT_PARAMETER;
        blocks[CONF_BLOCK_LED].defaultValue[ledLocalControlEnabled] = 0;
        blocks[CONF_BLOCK_LED].sectionParameters[ledLocalControlEnabled] = MAX_NUMBER_OF_LEDS;
    }

}
