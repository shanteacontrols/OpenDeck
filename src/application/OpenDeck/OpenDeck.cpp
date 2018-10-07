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

#include "OpenDeck.h"
#include "core/src/general/Timing.h"

#ifdef __AVR__
#include <util/atomic.h>

extern "C" void __cxa_pure_virtual()
{
    while (1);
}
#endif

static SysConfig    *sysConfRef = nullptr;
static Buttons      *buttonsRef = nullptr;

static bool sendCInfoCallback(dbBlockID_t dbBlock, sysExParameter_t componentID)
{
    return sysConfRef->sendCInfo(dbBlock, componentID);
}

static void analogButtonCallback(uint8_t analogIndex, uint16_t adcValue)
{
    buttonsRef->processButton(analogIndex+MAX_NUMBER_OF_BUTTONS, buttonsRef->getStateFromAnalogValue(adcValue));
}

#ifdef TOUCHSCREEN_SUPPORTED
static void touchButtonCallback(uint8_t index, bool state)
{
    buttonsRef->processButton(MAX_NUMBER_OF_BUTTONS+MAX_NUMBER_OF_ANALOG+index, state);
}
#endif

void OpenDeck::init()
{
    board.init();
    #ifdef USB_SUPPORTED
    #ifndef BOARD_A_xu2
    midi.handleUSBread(Board::usbReadMIDI);
    midi.handleUSBwrite(Board::usbWriteMIDI);
    #endif
    #endif
    database.init();

    sysConfig.init();

    //enable uart-to-usb link when usb isn't supported directly
    #ifndef USB_SUPPORTED
    sysConfig.setupMIDIoverUART_OD(UART_USB_LINK_CHANNEL);
    #endif

    #ifdef DIN_MIDI_SUPPORTED
    if (database.read(DB_BLOCK_MIDI, dbSection_midi_feature, midiFeatureDinEnabled))
    {
        sysConfig.setupMIDIoverUART(UART_MIDI_CHANNEL);

        //use recursive parsing when merging is active
        midi.useRecursiveParsing(database.read(DB_BLOCK_MIDI, dbSection_midi_feature, midiFeatureMergeEnabled));
    }
    #endif

    midi.setInputChannel(MIDI_CHANNEL_OMNI);
    midi.setNoteOffMode((noteOffType_t)database.read(DB_BLOCK_MIDI, dbSection_midi_feature, midiFeatureStandardNoteOff));
    midi.setRunningStatusState(database.read(DB_BLOCK_MIDI, dbSection_midi_feature, midiFeatureRunningStatus));
    midi.setChannelSendZeroStart(true);

    board.ledFlashStartup(board.checkNewRevision());

    #ifdef __AVR__
    //enable global interrupts
    sei();
    #endif

    #ifdef LEDS_SUPPORTED
    leds.init();
    #endif

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

    #ifdef BOARD_BERGAMOT
    touchscreen.init(ts_sdw);
    touchscreen.setPage(1);
    #endif

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

            sysConfig.sendCustomMessage(midi.usbMessage.sysexArray, daisyChainMessage, 1, false);
        }
    }
    #endif

    sysConfRef = &sysConfig;
    buttonsRef = &buttons;
    buttons.setCinfoHandler(sendCInfoCallback);
    encoders.setCinfoHandler(sendCInfoCallback);
    analog.setCinfoHandler(sendCInfoCallback);
    analog.setButtonHandler(analogButtonCallback);
    #ifdef TOUCHSCREEN_SUPPORTED
    touchscreen.setButtonHandler(touchButtonCallback);
    #endif
}

void OpenDeck::checkComponents()
{
    if (sysConfig.isProcessingEnabled())
    {
        if (board.digitalInputDataAvailable())
        {
            buttons.update();
            encoders.update();
        }

        analog.update();
        #ifdef LEDS_SUPPORTED
        leds.checkBlinking();
        #endif

        #ifdef DISPLAY_SUPPORTED
        display.update();
        #endif

        #ifdef TOUCHSCREEN_SUPPORTED
        touchscreen.update();
        #endif
    }
}

void OpenDeck::checkMIDI()
{
    //note: mega/uno
    //"fake" usbInterface - din data is stored as usb data so use usb callback to read the usb
    //packet stored in midi object
    if (midi.read(usbInterface))
    {
        //new message on usb
        midiMessageType_t messageType = midi.getType(usbInterface);
        #if defined(LEDS_SUPPORTED) || defined(DISPLAY_SUPPORTED)
        uint8_t data1 = midi.getData1(usbInterface);
        uint8_t data2 = midi.getData2(usbInterface);
        uint8_t channel = midi.getChannel(usbInterface);
        #endif

        switch(messageType)
        {
            case midiMessageSystemExclusive:
            //don't handle sysex on slave boards in daisy-chain setup
            #ifdef BOARD_OPEN_DECK
            if (board.isUSBconnected())
            #endif
                sysConfig.handleMessage(midi.getSysExArray(usbInterface), midi.getSysExArrayLength(usbInterface));
            break;

            case midiMessageNoteOn:
            case midiMessageNoteOff:
            case midiMessageControlChange:
            case midiMessageProgramChange:
            #if defined(LEDS_SUPPORTED) || defined(DISPLAY_SUPPORTED)
            if (messageType == midiMessageNoteOff)
                data2 = 0;
            #endif
            #ifdef LEDS_SUPPORTED
            leds.midiToState(messageType, data1, data2, channel);
            #endif
            #ifdef DISPLAY_SUPPORTED
            display.displayMIDIevent(displayEventIn, midiMessageNoteOn_display, data1, data2, channel+1);
            #endif
            break;

            case midiMessageClock:
            #ifdef LEDS_SUPPORTED
            leds.checkBlinking(true);
            #endif
            break;

            case midiMessageStart:
            #ifdef LEDS_SUPPORTED
            leds.resetBlinking();
            leds.checkBlinking(true);
            #endif
            break;

            default:
            break;
        }
    }

    #ifdef BOARD_OPEN_DECK
    if (database.read(DB_BLOCK_MIDI, dbSection_midi_feature, midiFeatureDinEnabled))
    {
        //check for incoming MIDI messages on UART
        //merging is supported on opendeck board only at the moment
        if (!database.read(DB_BLOCK_MIDI, dbSection_midi_feature, midiFeatureMergeEnabled))
        {
            //daisy-chained opendeck boards
            if (!board.getUARTloopbackState(UART_MIDI_CHANNEL))
            {
                if (!board.isUSBconnected())
                {
                    //in this case, board is slave, check only for sysex from master
                    if (midi.read(dinInterface))
                    {
                        midiMessageType_t messageType = midi.getType(dinInterface);

                        if (messageType == midiMessageSystemExclusive)
                        {
                            sysConfig.handleMessage(midi.getSysExArray(dinInterface), midi.getSysExArrayLength(dinInterface));
                        }
                    }
                }
                else
                {
                    //master opendeck - dump everything from MIDI in to USB MIDI out
                    USBMIDIpacket_t USBMIDIpacket;
                    if (Board::uartReadMIDI_OD(UART_MIDI_CHANNEL, USBMIDIpacket))
                        Board::usbWriteMIDI(USBMIDIpacket);
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
                //dump everything from DIN MIDI in to USB MIDI out
                midi.read(dinInterface, THRU_FULL_USB);
                break;

                default:
                break;
            }
        }
    }
    #endif
}

void OpenDeck::update()
{
    checkMIDI();
    checkComponents();
}