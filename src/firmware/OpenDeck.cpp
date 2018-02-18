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

#include "interface/Interface.h"
#include "Version.h"

MIDI midi;

void init()
{
    #if defined(BOARD_OPEN_DECK) || defined(BOARD_A_LEO) || defined(BOARD_A_PRO_MICRO) || defined(BOARD_T_2PP)
    midi.setUSBMIDIstate(true);
    #endif

    #if defined(BOARD_OPEN_DECK) || defined(BOARD_A_LEO) || defined(BOARD_A_PRO_MICRO) || defined(BOARD_T_2PP)
    midi.setDINMIDIstate(database.read(DB_BLOCK_MIDI, midiFeatureSection, midiFeatureDinEnabled));
    #endif

    #ifdef BOARD_A_MEGA
    //always enable (for now)
    midi.setDINMIDIstate(true);
    #endif

    #ifdef BOARD_A_UNO
    //always enable
    midi.setDINMIDIstate(true);
    #endif

    midi.setOneByteParseDINstate(true);
    midi.setInputChannel(database.read(DB_BLOCK_MIDI, midiChannelSection, midiChannelInput));
    midi.setNoteOffMode((noteOffType_t)database.read(DB_BLOCK_MIDI, midiFeatureSection, midiFeatureStandardNoteOff));
    midi.setRunningStatusState(database.read(DB_BLOCK_MIDI, midiFeatureSection, midiFeatureRunningStatus));

    database.init();
    board.init();
    sysEx.init();

    #if !defined(BOARD_A_MEGA) && !defined(BOARD_A_UNO)
    if (board.checkNewRevision())
    {
        for (int i=0; i<3; i++)
        {
            BTLDR_LED_ON(LED_OUT_PORT, LED_OUT_PIN);
            BTLDR_LED_OFF(LED_IN_PORT, LED_IN_PIN);
            //teensy has only one led - use longer delay to properly indicate
            //updated fw
            #ifdef BOARD_T_2PP
            _delay_ms(500);
            #else
            _delay_ms(200);
            #endif
            BTLDR_LED_OFF(LED_OUT_PORT, LED_OUT_PIN);
            BTLDR_LED_ON(LED_IN_PORT, LED_IN_PIN);
            #ifdef BOARD_T_2PP
            _delay_ms(500);
            #else
            _delay_ms(200);
            #endif
        }

        BTLDR_LED_OFF(LED_OUT_PORT, LED_OUT_PIN);
        BTLDR_LED_OFF(LED_IN_PORT, LED_IN_PIN);
    }
    else
    {
        BTLDR_LED_ON(LED_OUT_PORT, LED_OUT_PIN);
        BTLDR_LED_ON(LED_IN_PORT, LED_IN_PIN);
        _delay_ms(200);
        BTLDR_LED_OFF(LED_OUT_PORT, LED_OUT_PIN);
        BTLDR_LED_OFF(LED_IN_PORT, LED_IN_PIN);
        _delay_ms(200);
    }
    #endif

    //enable global interrupts
    sei();

    leds.init();

    // #ifdef DISPLAY_SUPPORTED
    // if (database.read(DB_BLOCK_DISPLAY, displayFeaturesSection, displayFeatureEnable))
    //     // display.init((displayController)database.read(DB_BLOCK_DISPLAY, displayHwSection, displayHwController), (displayResolution)database.read(DB_BLOCK_DISPLAY, displayHwSection, displayHwResolution));
    //     display.init(displayController_ssd1306, displayRes_128x32);
    // #endif
}

int main()
{
    init();

    #ifdef DISPLAY_SUPPORTED
    display.init(displayController_ssd1306, displayRes_128x64);
    // display.init(displayController_ssd1306, displayRes_128x32);
    display.displayHome();
    #endif

    while(1)
    {
        #if defined(BOARD_A_LEO) || defined(BOARD_OPEN_DECK) || defined(BOARD_A_PRO_MICRO) || defined(BOARD_T_2PP)
        if (midi.read(usbInterface))
        {
            //new message on usb
            midiMessageType_t messageType = midi.getType(usbInterface);
            uint8_t data1 = midi.getData1(usbInterface);
            uint8_t data2 = midi.getData2(usbInterface);

            switch(messageType)
            {
                case midiMessageSystemExclusive:
                sysEx.handleMessage(midi.getSysExArray(usbInterface), midi.getSysExArrayLength(usbInterface));
                break;

                case midiMessageNoteOn:
                //we're using received note data to control LED color
                leds.noteToState(data1, data2);
                #ifdef DISPLAY_SUPPORTED
                display.displayMIDIevent(displayEventIn, midiMessageNoteOn_display, data1, data2, database.read(DB_BLOCK_MIDI, midiChannelSection, midiChannelInput));
                #endif
                break;

                case midiMessageNoteOff:
                //always turn led off when note off is received
                leds.noteToState(data1, 0);
                #ifdef DISPLAY_SUPPORTED
                display.displayMIDIevent(displayEventIn, midiMessageNoteOff_display, data1, data2, database.read(DB_BLOCK_MIDI, midiChannelSection, midiChannelInput));
                #endif
                break;

                case midiMessageControlChange:
                //control change is used to control led blinking
                leds.ccToBlink(data1, data2);
                #ifdef DISPLAY_SUPPORTED
                display.displayMIDIevent(displayEventIn, midiMessageControlChange_display, data1, data2, database.read(DB_BLOCK_MIDI, midiChannelSection, midiChannelInput));
                #endif
                break;

                default:
                break;
            }
        }
        #endif

        #if !defined(BOARD_A_MEGA) && !defined(BOARD_A_UNO)
        if (midi.getDINMIDIstate())
        {
            //check for incoming MIDI messages on USART
            if (!database.read(DB_BLOCK_MIDI, midiFeatureSection, midiFeatureUSBconvert))
            {
                midi.read(dinInterface);
                midiMessageType_t messageType = midi.getType(dinInterface);
                uint8_t data1 = midi.getData1(dinInterface);
                uint8_t data2 = midi.getData2(dinInterface);

                switch(messageType)
                {
                    case midiMessageNoteOff:
                    case midiMessageNoteOn:
                    leds.noteToState(data1, data2);
                    break;

                    default:
                    break;
                }
            }
            else
            {
                //dump everything from MIDI in to USB MIDI out
                midi.read(dinInterface, THRU_FULL_USB);
            }
        }
        #endif

        #if defined(BOARD_A_MEGA) || defined(BOARD_A_UNO)
        //din interface here is actually translated usb traffic from 16u2
        if (midi.read(dinInterface))
        {
            midiMessageType_t messageType = midi.getType(dinInterface);
            uint8_t data1 = midi.getData1(dinInterface);
            uint8_t data2 = midi.getData2(dinInterface);

            switch(messageType)
            {
                case midiMessageSystemExclusive:
                sysEx.handleMessage(midi.getSysExArray(dinInterface), midi.getSysExArrayLength(dinInterface));
                break;

                case midiMessageNoteOn:
                //we're using received note data to control LED color
                leds.noteToState(data1, data2);
                #ifdef DISPLAY_SUPPORTED
                display.displayMIDIevent(displayEventIn, midiMessageNoteOn_display, data1, data2, database.read(DB_BLOCK_MIDI, midiChannelSection, midiChannelInput));
                #endif
                break;

                case midiMessageNoteOff:
                //always turn led off when note off is received
                leds.noteToState(data1, 0);
                #ifdef DISPLAY_SUPPORTED
                display.displayMIDIevent(displayEventIn, midiMessageNoteOff_display, data1, data2, database.read(DB_BLOCK_MIDI, midiChannelSection, midiChannelInput));
                #endif
                break;

                case midiMessageControlChange:
                //control change is used to control led blinking
                leds.ccToBlink(data1, data2);
                #ifdef DISPLAY_SUPPORTED
                display.displayMIDIevent(displayEventIn, midiMessageControlChange_display, data1, data2, database.read(DB_BLOCK_MIDI, midiChannelSection, midiChannelInput));
                #endif
                break;

                default:
                break;
            }
        }
        #endif

        digitalInput.update();
        analog.update();
        leds.update();

        #ifdef DISPLAY_SUPPORTED
        display.update();
        #endif
    }
}
