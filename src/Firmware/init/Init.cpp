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
    sysEx.addBlocks(DB_BLOCKS);

    sysExSection section;

    {
        //MIDI block

        //midi feature section
        section.numberOfParameters = MIDI_FEATURES;
        section.minValue = 0;
        section.maxValue = 1;

        sysEx.addSection(DB_BLOCK_MIDI, section);

        //midi channel section
        section.numberOfParameters = MIDI_CHANNELS;
        section.minValue = 1;
        section.maxValue = 16;

        sysEx.addSection(DB_BLOCK_MIDI, section);
    }

    {
        //button block

        //type section
        section.numberOfParameters = MAX_NUMBER_OF_BUTTONS+MAX_NUMBER_OF_ANALOG;
        section.minValue = 0;
        section.maxValue = BUTTON_TYPES-1;

        sysEx.addSection(DB_BLOCK_BUTTON, section);

        //program change enabled section
        section.numberOfParameters = MAX_NUMBER_OF_BUTTONS+MAX_NUMBER_OF_ANALOG;
        section.minValue = 0;
        section.maxValue = 1;

        sysEx.addSection(DB_BLOCK_BUTTON, section);

        //midi id section
        section.numberOfParameters = MAX_NUMBER_OF_BUTTONS+MAX_NUMBER_OF_ANALOG;
        section.minValue = 0;
        section.maxValue = 127;

        sysEx.addSection(DB_BLOCK_BUTTON, section);
    }

    {
        //encoder block

        //encoder enabled section
        section.numberOfParameters = MAX_NUMBER_OF_ENCODERS;
        section.minValue = 0;
        section.maxValue = 1;

        sysEx.addSection(DB_BLOCK_ENCODER, section);

        //encoder inverted section
        section.numberOfParameters = MAX_NUMBER_OF_ENCODERS;
        section.minValue = 0;
        section.maxValue = 1;

        sysEx.addSection(DB_BLOCK_ENCODER, section);

        //encoding mode section
        section.numberOfParameters = MAX_NUMBER_OF_ENCODERS;
        section.minValue = 0;
        section.maxValue = ENCODING_MODES-1;

        sysEx.addSection(DB_BLOCK_ENCODER, section);

        //midi id section
        section.numberOfParameters = MAX_NUMBER_OF_ENCODERS;
        section.minValue = 0;
        section.maxValue = 127;

        sysEx.addSection(DB_BLOCK_ENCODER, section);
    }

    {
        //analog block

        //analog enabled section
        section.numberOfParameters = MAX_NUMBER_OF_ANALOG;
        section.minValue = 0;
        section.maxValue = 1;

        sysEx.addSection(DB_BLOCK_ANALOG, section);

        //analog inverted section
        section.numberOfParameters = MAX_NUMBER_OF_ANALOG;
        section.minValue = 0;
        section.maxValue = 1;

        sysEx.addSection(DB_BLOCK_ANALOG, section);

        //analog type section
        section.numberOfParameters = MAX_NUMBER_OF_ANALOG;
        section.minValue = 0;
        section.maxValue = ANALOG_TYPES-1;

        sysEx.addSection(DB_BLOCK_ANALOG, section);

        //midi id section
        section.numberOfParameters = MAX_NUMBER_OF_ANALOG;
        section.minValue = 0;
        section.maxValue = 127;

        sysEx.addSection(DB_BLOCK_ANALOG, section);

        //lower cc limit
        section.numberOfParameters = MAX_NUMBER_OF_ANALOG;
        section.minValue = 0;
        section.maxValue = 127;

        sysEx.addSection(DB_BLOCK_ANALOG, section);

        //upper cc limit
        section.numberOfParameters = MAX_NUMBER_OF_ANALOG;
        section.minValue = 0;
        section.maxValue = 127;

        sysEx.addSection(DB_BLOCK_ANALOG, section);
    }

    {
        //led block

        //hardware parameters section
        section.numberOfParameters = LED_HARDWARE_PARAMETERS;
        section.minValue = 0;
        section.maxValue = 0;

        sysEx.addSection(DB_BLOCK_ANALOG, section);

        //activation note section
        section.numberOfParameters = MAX_NUMBER_OF_LEDS;
        section.minValue = 0;
        section.maxValue = 127;

        sysEx.addSection(DB_BLOCK_ANALOG, section);

        //rgb enabled section
        section.numberOfParameters = MAX_NUMBER_OF_LEDS;
        section.minValue = 0;
        section.maxValue = 1;

        sysEx.addSection(DB_BLOCK_ANALOG, section);

        //local led control enabled section
        section.numberOfParameters = MAX_NUMBER_OF_LEDS;
        section.minValue = 0;
        section.maxValue = 1;

        sysEx.addSection(DB_BLOCK_ANALOG, section);

        //led color section
        section.numberOfParameters = MAX_NUMBER_OF_LEDS;
        section.minValue = 0;
        section.maxValue = LED_COLORS-1;

        sysEx.addSection(DB_BLOCK_ANALOG, section);

        //led blink section
        section.numberOfParameters = MAX_NUMBER_OF_LEDS;
        section.minValue = 0;
        section.maxValue = 1;

        sysEx.addSection(DB_BLOCK_ANALOG, section);
    }
}

void globalInit()
{
    midi.init(usbInterface);
    midi.init(dinInterface);

    database.init();
    board.init();

    midi.setInputChannel(database.read(DB_BLOCK_MIDI, midiChannelSection, inputChannel));
    midi.setNoteChannel(database.read(DB_BLOCK_MIDI, midiChannelSection, noteChannel));
    midi.setCCchannel(database.read(DB_BLOCK_MIDI, midiChannelSection, CCchannel));
    midi.setProgramChangeChannel(database.read(DB_BLOCK_MIDI, midiChannelSection, programChangeChannel));

    initSysEx();

    if (checkNewRevision())
    {
        for (int i=0; i<3; i++)
        {
            setHigh(LED_OUT_PORT, LED_OUT_PIN);
            setLow(LED_IN_PORT, LED_IN_PIN);
            _delay_ms(200);
            setLow(LED_OUT_PORT, LED_OUT_PIN);
            setHigh(LED_IN_PORT, LED_IN_PIN);
            _delay_ms(200);
        }

        setLow(LED_OUT_PORT, LED_OUT_PIN);
        setLow(LED_IN_PORT, LED_IN_PIN);
    }
    else
    {
        setHigh(LED_OUT_PORT, LED_OUT_PIN);
        setHigh(LED_IN_PORT, LED_IN_PIN);
        _delay_ms(200);
        setLow(LED_OUT_PORT, LED_OUT_PIN);
        setLow(LED_IN_PORT, LED_IN_PIN);
        _delay_ms(200);
    }

    leds.init();

    //enable global interrupts
    sei();
}
