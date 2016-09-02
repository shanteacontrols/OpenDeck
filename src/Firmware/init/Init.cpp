#include "Init.h"

void initSysEx()    {

    for (int i=0; i<CONF_BLOCKS; i++)
        sysEx.addBlock(configuration.getBlockSections(i));

    {
        const sysExSection midiFeature_section = { MIDI_FEATURES, 0, 1 };
        const sysExSection midiChannel_section = { MIDI_CHANNELS, 1, 16 };

        const sysExSection *midiSubtypeArray[] = {

            &midiFeature_section,
            &midiChannel_section

        };

        for (int i=0; i<MIDI_SECTIONS; i++) {

            sysEx.addSection(CONF_BLOCK_MIDI, midiSubtypeArray[i]->numberOfParameters, midiSubtypeArray[i]->minValue, midiSubtypeArray[i]->maxValue);

        }
    }

    {
        const sysExSection buttonType_section                   = { MAX_NUMBER_OF_BUTTONS, 0, BUTTON_TYPES-1 };
        const sysExSection buttonProgramChangeEnabled_section   = { MAX_NUMBER_OF_BUTTONS, 0, 1 };
        const sysExSection buttonMIDIid_section                 = { MAX_NUMBER_OF_BUTTONS, 0, 127 };

        const sysExSection *buttonSubtypeArray[] = {

            &buttonType_section,
            &buttonProgramChangeEnabled_section,
            &buttonMIDIid_section

        };

        for (int i=0; i<BUTTON_SECTIONS; i++)   {

            sysEx.addSection(CONF_BLOCK_BUTTON, buttonSubtypeArray[i]->numberOfParameters, buttonSubtypeArray[i]->minValue, buttonSubtypeArray[i]->maxValue);

        }
    }

    {
        const sysExSection encoderEnabled_section           = { MAX_NUMBER_OF_ENCODERS, 0, 1 };
        const sysExSection encoderInverted_section          = { MAX_NUMBER_OF_ENCODERS, 0, 1 };
        const sysExSection encoderEncodingMode_section      = { MAX_NUMBER_OF_ENCODERS, 0, ENCODING_MODES-1 };
        const sysExSection encoderMIDIid_section            = { MAX_NUMBER_OF_ENCODERS, 0, 127 };

        const sysExSection *encodersSubtypeArray[] = {

            &encoderEnabled_section,
            &encoderInverted_section,
            &encoderEncodingMode_section,
            &encoderMIDIid_section

        };

        for (int i=0; i<ENCODER_SECTIONS; i++)  {

            sysEx.addSection(CONF_BLOCK_ENCODER, encodersSubtypeArray[i]->numberOfParameters, encodersSubtypeArray[i]->minValue, encodersSubtypeArray[i]->maxValue);

        }
    }

    {
        const sysExSection analogEnabled_section            = { MAX_NUMBER_OF_ANALOG, 0, 1 };
        const sysExSection analogInverted_section           = { MAX_NUMBER_OF_ANALOG, 0, 1 };
        const sysExSection analogType_section               = { MAX_NUMBER_OF_ANALOG, 0, ANALOG_TYPES-1 };
        const sysExSection analogMIDIid_section             = { MAX_NUMBER_OF_ANALOG, 0, 127 };
        const sysExSection analogCClowerLimit_section       = { MAX_NUMBER_OF_ANALOG, 0, 127 };
        const sysExSection analogCCupperLimit_section       = { MAX_NUMBER_OF_ANALOG, 0, 127 };

        const sysExSection *analogSubtypeArray[] = {

            &analogEnabled_section,
            &analogType_section,
            &analogInverted_section,
            &analogMIDIid_section,
            &analogCClowerLimit_section,
            &analogCCupperLimit_section

        };

        for (int i=0; i<ANALOG_SECTIONS; i++)   {

            sysEx.addSection(CONF_BLOCK_ANALOG, analogSubtypeArray[i]->numberOfParameters, analogSubtypeArray[i]->minValue, analogSubtypeArray[i]->maxValue);

        }
    }

    {
        const sysExSection ledHardwareParameter_section     = { LED_HARDWARE_PARAMETERS, 0, 0 };
        const sysExSection ledActivationNote_section        = { MAX_NUMBER_OF_LEDS, 0, 127 };
        const sysExSection ledStartUpNumber_section         = { MAX_NUMBER_OF_LEDS, 0, MAX_NUMBER_OF_LEDS-1 };
        const sysExSection ledRGBenabled_section            = { MAX_NUMBER_OF_RGB_LEDS, 0, 1 };
        //extra - not defined in eeprom blocks since it's not persistent section
        const sysExSection ledState_section                 = { MAX_NUMBER_OF_LEDS, 0, LED_STATES-1 };

        const sysExSection *ledsSubtypeArray[] = {

            &ledHardwareParameter_section,
            &ledActivationNote_section,
            &ledStartUpNumber_section,
            &ledRGBenabled_section,
            &ledState_section

        };

        for (int i=0; i<LED_SECTIONS+1; i++)  {

            sysEx.addSection(CONF_BLOCK_LED, ledsSubtypeArray[i]->numberOfParameters, ledsSubtypeArray[i]->minValue, ledsSubtypeArray[i]->maxValue);

        }

    }

}

void globalInit()   {

    configuration.init();
    core.init();
    midi.init();
    leds.init();
    initSysEx();
    checkNewRevision();

}