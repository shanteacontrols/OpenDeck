/*
    OpenDeck MIDI platform firmware
    Copyright (C) 2015-2018 Igor Petrovic

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
bool processingEnabled;

void init()
{
    board.init();
    database.init();
    sysEx.init();

    //use recursive parsing when merging is active
    if (database.read(DB_BLOCK_MIDI, dbSection_midi_feature, midiFeatureMergeEnabled))
        midi.setOneByteParseDINstate(false);
    else
        midi.setOneByteParseDINstate(true);

    midi.setInputChannel(MIDI_CHANNEL_OMNI);
    midi.setNoteOffMode((noteOffType_t)database.read(DB_BLOCK_MIDI, dbSection_midi_feature, midiFeatureStandardNoteOff));
    midi.setRunningStatusState(database.read(DB_BLOCK_MIDI, dbSection_midi_feature, midiFeatureRunningStatus));
    midi.setChannelSendZeroStart(true);

    board.ledFlashStartup(board.checkNewRevision());

    //enable global interrupts
    sei();

    leds.init();

    #ifdef DISPLAY_SUPPORTED
    if (database.read(DB_BLOCK_DISPLAY, dbSection_display_features, displayFeatureEnable))
    {
        if (display.init((displayController_t)database.read(DB_BLOCK_DISPLAY, dbSection_display_hw, displayHwController), (displayResolution_t)database.read(DB_BLOCK_DISPLAY, dbSection_display_hw, displayHwResolution)))
        {
            display.setDirectWriteState(true);

            if (database.read(DB_BLOCK_DISPLAY, dbSection_display_features, displayFeatureWelcomeMsg))
                display.displayWelcomeMessage();

            if (database.read(DB_BLOCK_DISPLAY, dbSection_display_features, displayFeatureVInfoMsg))
                display.displayVinfo(false);

            display.setDirectWriteState(false);

            display.displayHome();
            display.setRetentionState(database.read(DB_BLOCK_DISPLAY, dbSection_display_features, displayFeatureMIDIeventRetention));
            display.setRetentionTime(database.read(DB_BLOCK_DISPLAY, dbSection_display_features, displayFeatureMIDIeventTime) * 1000);
            display.setAlternateNoteDisplay(database.read(DB_BLOCK_DISPLAY, dbSection_display_features, displayFeatureMIDInotesAlternate));
        }
    }
    #endif

    processingEnabled = true;
}

int main()
{
    init();

    #ifdef USB_SUPPORTED
    //wait a bit before checking if usb is connected
    wait_ms(1500);

    if (board.isUSBconnected())
    {
        //this board is connected to usb, check if din is enabled
        if (database.read(DB_BLOCK_MIDI, dbSection_midi_feature, midiFeatureDinEnabled) && !database.read(DB_BLOCK_MIDI, dbSection_midi_feature, midiFeatureMergeEnabled))
        {
            //start daisy-chain auto-config
            //format message as special request so that it's parsed on other boards
            sysExParameter_t daisyChainMessage[] =
            {
                SYSEX_CR_DAISY_CHAIN_MASTER,
            };

            sysEx.sendCustomMessage(usbMessage.sysexArray, daisyChainMessage, 1, false);
        }
    }
    #endif

    while(1)
    {
        #if defined(USB_SUPPORTED) || defined(BOARD_A_MEGA) || defined(BOARD_A_UNO)
        //note: mega/uno
        //"fake" usbInterface - din data is stored as usb data so use usb callback to read the usb
        //packet stored in midi object
        if (midi.read(usbInterface))
        {
            //new message on usb
            midiMessageType_t messageType = midi.getType(usbInterface);
            uint8_t data1 = midi.getData1(usbInterface);
            uint8_t data2 = midi.getData2(usbInterface);
            uint8_t channel = midi.getChannel(usbInterface);

            switch(messageType)
            {
                case midiMessageSystemExclusive:
                sysEx.handleMessage(midi.getSysExArray(usbInterface), midi.getSysExArrayLength(usbInterface));
                break;

                case midiMessageNoteOn:
                //we're using received note data to control LED color
                leds.noteToState(data1, data2, channel);
                #ifdef DISPLAY_SUPPORTED
                display.displayMIDIevent(displayEventIn, midiMessageNoteOn_display, data1, data2, channel+1);
                #endif
                break;

                case midiMessageNoteOff:
                //always turn led off when note off is received
                leds.noteToState(data1, 0, channel);
                #ifdef DISPLAY_SUPPORTED
                display.displayMIDIevent(displayEventIn, midiMessageNoteOff_display, data1, data2, channel+1);
                #endif
                break;

                case midiMessageControlChange:
                //control change is used to control led blinking
                leds.ccToBlink(data1, data2, channel);
                #ifdef DISPLAY_SUPPORTED
                display.displayMIDIevent(displayEventIn, midiMessageControlChange_display, data1, data2, channel+1);
                #endif
                break;

                default:
                break;
            }
        }
        #endif

        #if !defined(BOARD_A_MEGA) && !defined(BOARD_A_UNO)
        if (database.read(DB_BLOCK_MIDI, dbSection_midi_feature, midiFeatureDinEnabled))
        {
            //check for incoming MIDI messages on USART
            if (!database.read(DB_BLOCK_MIDI, dbSection_midi_feature, midiFeatureMergeEnabled))
            {
                //daisy-chained opendeck boards
                if (!board.getUARTloopbackState())
                {
                    if (!board.isUSBconnected())
                    {
                        //in this case, board is slave, check only for sysex from master
                        if (midi.read(dinInterface))
                        {
                            midiMessageType_t messageType = midi.getType(dinInterface);

                            if (messageType == midiMessageSystemExclusive)
                            {
                                sysEx.handleMessage(midi.getSysExArray(dinInterface), midi.getSysExArrayLength(dinInterface));
                            }
                        }
                    }
                    else
                    {
                        //master opendeck - dump everything from MIDI in to USB MIDI out
                        if (board.MIDIread_UART_OD())
                            board.MIDIwrite_USB(usbMIDIpacket);
                    }
                }
                else
                {
                    //all incoming data is forwarded automatically (inner slave)
                }
            }
            else
            {
                switch(database.read(DB_BLOCK_MIDI, dbSection_midi_merge, midiMergeToInterface))
                {
                    case midiMergeToInterfaceUSB:
                    //dump everything from MIDI in to USB MIDI out
                    midi.read(dinInterface, THRU_FULL_USB);
                    break;

                    case midiMergeToInterfaceDIN:
                    //dump everything from MIDI in to MIDI out
                    midi.read(dinInterface, THRU_FULL_DIN);
                    break;

                    case midiMergeToInterfaceAll:
                    //dump everything from MIDI in to USB MIDI out and MIDI out
                    midi.read(dinInterface, THRU_FULL_ALL);
                    break;

                    default:
                    break;
                }
            }
        }
        #endif

        if (processingEnabled)
        {
            digitalInput.update();
            analog.update();
            leds.update();
        }

        #ifdef DISPLAY_SUPPORTED
        display.update();
        #endif
    }
}
