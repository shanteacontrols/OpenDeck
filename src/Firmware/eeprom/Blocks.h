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
    analogDigitalEnabledSection,
    ANALOG_SECTIONS

} analogSection;

typedef enum {

    ledHardwareParameterSection,
    ledActivationNoteSection,
    ledStartUpNumberSection,
    ledRGBenabledSection,
    ledLocalControlEnabled,
    LED_SECTIONS

} ledSection;

#endif