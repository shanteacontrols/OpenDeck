/*
    OpenDeck MIDI platform firmware
    Copyright (C) 2015-2017 Igor Petrovic

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

#include "Init.h"

void initSysEx()
{
    {
        sysEx.addBlock(MIDI_SECTIONS);

        const sysExSection midiFeature_section = { MIDI_FEATURES, 0, 1 };
        const sysExSection midiChannel_section = { MIDI_CHANNELS, 1, 16 };

        const sysExSection *midiSectionArray[] =
        {
            &midiFeature_section,
            &midiChannel_section
        };

        for (int i=0; i<MIDI_SECTIONS; i++)
            sysEx.addSection(CONF_BLOCK_MIDI, midiSectionArray[i]->numberOfParameters, midiSectionArray[i]->minValue, midiSectionArray[i]->maxValue);
    }

    {
        sysEx.addBlock(BUTTON_SECTIONS);

        const sysExSection buttonType_section                   = { MAX_NUMBER_OF_BUTTONS+MAX_NUMBER_OF_ANALOG, 0, BUTTON_TYPES-1 };
        const sysExSection buttonProgramChangeEnabled_section   = { MAX_NUMBER_OF_BUTTONS+MAX_NUMBER_OF_ANALOG, 0, 1 };
        const sysExSection buttonMIDIid_section                 = { MAX_NUMBER_OF_BUTTONS+MAX_NUMBER_OF_ANALOG, 0, 127 };

        const sysExSection *buttonSectionArray[] =
        {
            &buttonType_section,
            &buttonProgramChangeEnabled_section,
            &buttonMIDIid_section
        };

        for (int i=0; i<BUTTON_SECTIONS; i++)
            sysEx.addSection(CONF_BLOCK_BUTTON, buttonSectionArray[i]->numberOfParameters, buttonSectionArray[i]->minValue, buttonSectionArray[i]->maxValue);
    }

    {
        sysEx.addBlock(ENCODER_SECTIONS);

        const sysExSection encoderEnabled_section           = { MAX_NUMBER_OF_ENCODERS, 0, 1 };
        const sysExSection encoderInverted_section          = { MAX_NUMBER_OF_ENCODERS, 0, 1 };
        const sysExSection encoderEncodingMode_section      = { MAX_NUMBER_OF_ENCODERS, 0, ENCODING_MODES-1 };
        const sysExSection encoderMIDIid_section            = { MAX_NUMBER_OF_ENCODERS, 0, 127 };

        const sysExSection *encodersSectionArray[] =
        {
            &encoderEnabled_section,
            &encoderInverted_section,
            &encoderEncodingMode_section,
            &encoderMIDIid_section
        };

        for (int i=0; i<ENCODER_SECTIONS; i++)
            sysEx.addSection(CONF_BLOCK_ENCODER, encodersSectionArray[i]->numberOfParameters, encodersSectionArray[i]->minValue, encodersSectionArray[i]->maxValue);
    }

    {
        sysEx.addBlock(ANALOG_SECTIONS);

        const sysExSection analogEnabled_section            = { MAX_NUMBER_OF_ANALOG, 0, 1 };
        const sysExSection analogInverted_section           = { MAX_NUMBER_OF_ANALOG, 0, 1 };
        const sysExSection analogType_section               = { MAX_NUMBER_OF_ANALOG, 0, ANALOG_TYPES-1 };
        const sysExSection analogMIDIid_section             = { MAX_NUMBER_OF_ANALOG, 0, 127 };
        const sysExSection analogCClowerLimit_section       = { MAX_NUMBER_OF_ANALOG, 0, 127 };
        const sysExSection analogCCupperLimit_section       = { MAX_NUMBER_OF_ANALOG, 0, 127 };

        const sysExSection *analogSectionArray[] =
        {
            &analogEnabled_section,
            &analogInverted_section,
            &analogType_section,
            &analogMIDIid_section,
            &analogCClowerLimit_section,
            &analogCCupperLimit_section
        };

        for (int i=0; i<ANALOG_SECTIONS; i++)
            sysEx.addSection(CONF_BLOCK_ANALOG, analogSectionArray[i]->numberOfParameters, analogSectionArray[i]->minValue, analogSectionArray[i]->maxValue);
    }

    {
        sysEx.addBlock(LED_SECTIONS+2);

        const sysExSection ledHardwareParameter_section     = { LED_HARDWARE_PARAMETERS, 0, 0 };
        const sysExSection ledActivationNote_section        = { MAX_NUMBER_OF_LEDS, 0, MAX_NUMBER_OF_LEDS }; //MAX_NUMBER_OF_LEDS = blank note
        const sysExSection ledRGBenabled_section            = { MAX_NUMBER_OF_LEDS, 0, 1 };
        const sysExSection ledLocalControl_Enabled          = { MAX_NUMBER_OF_LEDS, 0, 1 };
        //extras - not defined in eeprom blocks since it's not persistent section
        const sysExSection ledColor_section                 = { MAX_NUMBER_OF_LEDS, 0, LED_COLORS-1 };
        const sysExSection ledBlink_section                 = { MAX_NUMBER_OF_LEDS, 0, 1 };

        const sysExSection *ledsSectionArray[] =
        {
            &ledHardwareParameter_section,
            &ledActivationNote_section,
            &ledRGBenabled_section,
            &ledLocalControl_Enabled,
            &ledColor_section,
            &ledBlink_section
        };

        for (int i=0; i<(LED_SECTIONS+2); i++)
            sysEx.addSection(CONF_BLOCK_LED, ledsSectionArray[i]->numberOfParameters, ledsSectionArray[i]->minValue, ledsSectionArray[i]->maxValue);
    }
}

void globalInit()
{
    midi.init(usbInterface);
    midi.init(dinInterface);

    database.init();
    board.init();

    midi.setInputChannel(database.read(CONF_BLOCK_MIDI, midiChannelSection, inputChannel));
    midi.setNoteChannel(database.read(CONF_BLOCK_MIDI, midiChannelSection, noteChannel));
    midi.setCCchannel(database.read(CONF_BLOCK_MIDI, midiChannelSection, CCchannel));
    midi.setProgramChangeChannel(database.read(CONF_BLOCK_MIDI, midiChannelSection, programChangeChannel));

    initSysEx();
    checkNewRevision();

    leds.init();
}
