#include "Init.h"

void initSysEx()    {

    for (int i=0; i<CONF_BLOCKS; i++)
        sysEx.addBlock(configuration.getBlockSections(i));

    {
        const sysExSection midiFeatureSubtype = { MIDI_FEATURES, 0, 1 };
        const sysExSection midiChannelSubtype = { MIDI_CHANNELS, 1, 16 };

        const sysExSection *midiSubtypeArray[] = {

            &midiFeatureSubtype,
            &midiChannelSubtype

        };

        for (int i=0; i<MIDI_SUBTYPES; i++) {

            sysEx.addSection(CONF_BLOCK_MIDI, midiSubtypeArray[i]->numberOfParameters, midiSubtypeArray[i]->minValue, midiSubtypeArray[i]->maxValue);

        }
    }

    {
        const sysExSection buttonTypeSubtype                   = { MAX_NUMBER_OF_BUTTONS, 0, BUTTON_TYPES-1 };
        const sysExSection buttonProgramChangeEnabledSubtype   = { MAX_NUMBER_OF_BUTTONS, 0, 1 };
        const sysExSection buttonMIDIidSubtype                 = { MAX_NUMBER_OF_BUTTONS, 0, 127 };

        const sysExSection *buttonSubtypeArray[] = {

            &buttonTypeSubtype,
            &buttonProgramChangeEnabledSubtype,
            &buttonMIDIidSubtype

        };

        for (int i=0; i<BUTTON_SUBTYPES; i++)   {

            sysEx.addSection(CONF_BLOCK_BUTTON, buttonSubtypeArray[i]->numberOfParameters, buttonSubtypeArray[i]->minValue, buttonSubtypeArray[i]->maxValue);

        }
    }

    {
        const sysExSection encoderEnabledSubtype       = { MAX_NUMBER_OF_ENCODERS, 0, 1 };
        const sysExSection encoderInvertedSubtype      = { MAX_NUMBER_OF_ENCODERS, 0, 1 };
        const sysExSection encoderEncodingModeSubtype  = { MAX_NUMBER_OF_ENCODERS, 0, ENCODING_MODES-1 };
        const sysExSection encoderMIDIidSubtype        = { MAX_NUMBER_OF_ENCODERS, 0, 127 };

        const sysExSection *encodersSubtypeArray[] = {

            &encoderEnabledSubtype,
            &encoderInvertedSubtype,
            &encoderEncodingModeSubtype,
            &encoderMIDIidSubtype

        };

        for (int i=0; i<ENCODER_SUBTYPES; i++)  {

            sysEx.addSection(CONF_BLOCK_ENCODER, encodersSubtypeArray[i]->numberOfParameters, encodersSubtypeArray[i]->minValue, encodersSubtypeArray[i]->maxValue);

        }
    }

    {
        const sysExSection analogEnabledSubtype       = { MAX_NUMBER_OF_ANALOG, 0, 1 };
        const sysExSection analogInvertedSubtype      = { MAX_NUMBER_OF_ANALOG, 0, 1 };
        const sysExSection analogTypeSubtype          = { MAX_NUMBER_OF_ANALOG, 0, ANALOG_TYPES-1 };
        const sysExSection analogMIDIidSubtype        = { MAX_NUMBER_OF_ANALOG, 0, 127 };
        const sysExSection analogCClowerLimitSubtype  = { MAX_NUMBER_OF_ANALOG, 0, 127 };
        const sysExSection analogCCupperLimitSubtype  = { MAX_NUMBER_OF_ANALOG, 0, 127 };

        const sysExSection *analogSubtypeArray[] = {

            &analogEnabledSubtype,
            &analogTypeSubtype,
            &analogInvertedSubtype,
            &analogMIDIidSubtype,
            &analogCClowerLimitSubtype,
            &analogCCupperLimitSubtype

        };

        for (int i=0; i<ANALOG_SUBTYPES; i++)   {

            sysEx.addSection(CONF_BLOCK_ANALOG, analogSubtypeArray[i]->numberOfParameters, analogSubtypeArray[i]->minValue, analogSubtypeArray[i]->maxValue);

        }
    }

    {
        const sysExSection ledHardwareParameterSubtype   = { LED_HARDWARE_PARAMETERS, 0, 0 };
        const sysExSection ledActivationNoteSubtype      = { MAX_NUMBER_OF_LEDS, 0, 127 };
        const sysExSection ledStartUpNumberSubtype       = { MAX_NUMBER_OF_LEDS, 0, MAX_NUMBER_OF_LEDS-1 };
        const sysExSection ledRGBenabledSubtype          = { MAX_NUMBER_OF_RGB_LEDS, 0, 1 };
        const sysExSection ledsStateSubtype              = { MAX_NUMBER_OF_LEDS, 0, LED_STATES-1 };

        const sysExSection *ledsSubtypeArray[] = {

            &ledHardwareParameterSubtype,
            &ledActivationNoteSubtype,
            &ledStartUpNumberSubtype,
            &ledRGBenabledSubtype,
            &ledsStateSubtype

        };

        for (int i=0; i<LED_SUBTYPES; i++)  {

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