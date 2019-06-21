/*

Copyright 2015-2019 Igor Petrovic

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*/

#include "OpenDeck.h"
#include "core/src/general/Timing.h"
#include "interface/CInfo.h"

ComponentInfo cinfo;
Database database(Board::eeprom::read, Board::eeprom::write, EEPROM_SIZE-3);
MIDI midi;
#ifdef DISPLAY_SUPPORTED
Interface::Display display;
#endif
#ifdef TOUCHSCREEN_SUPPORTED
//assume sdw only for now
#include "interface/display/touch/model/sdw/SDW.h"
SDW sdw;
Interface::Touchscreen touchscreen(sdw);
#endif
#ifdef LEDS_SUPPORTED
Interface::digital::output::LEDs leds(database);
#endif
#ifdef LEDS_SUPPORTED
#ifdef DISPLAY_SUPPORTED
Interface::analog::Analog analog(database, midi, leds, display, cinfo);
#else
Interface::analog::Analog analog(database, midi, leds, cinfo);
#endif
#else
#ifdef DISPLAY_SUPPORTED
Interface::analog::Analog analog(database, midi, display, cinfo);
#else
Interface::analog::Analog analog(database, midi, cinfo);
#endif
#endif
#ifdef LEDS_SUPPORTED
#ifdef DISPLAY_SUPPORTED
Interface::digital::input::Buttons buttons(database, midi, leds, display, cinfo);
#else
Interface::digital::input::Buttons buttons(database, midi, leds, cinfo);
#endif
#else
#ifdef DISPLAY_SUPPORTED
Interface::digital::input::Buttons buttons(database, midi, display, cinfo);
#else
Interface::digital::input::Buttons buttons(database, midi, cinfo);
#endif
#endif
#ifdef LEDS_SUPPORTED
#ifdef DISPLAY_SUPPORTED
Interface::digital::input::Encoders encoders(database, midi, leds, display, cinfo);
#else
Interface::digital::input::Encoders encoders(database, midi, leds, cinfo);
#endif
#else
#ifdef DISPLAY_SUPPORTED
Interface::digital::input::Encoders encoders(database, midi, display, cinfo);
#else
Interface::digital::input::Encoders encoders(database, midi, cinfo);
#endif
#endif
#ifdef LEDS_SUPPORTED
#ifdef DISPLAY_SUPPORTED
SysConfig sysConfig(database, midi, buttons, encoders, analog, leds, display);
#else
SysConfig sysConfig(database, midi, buttons, encoders, analog, leds);
#endif
#else
#ifdef DISPLAY_SUPPORTED
SysConfig sysConfig(database, midi, buttons, encoders, analog, display);
#else
SysConfig sysConfig(database, midi, buttons, encoders, analog);
#endif
#endif

void OpenDeck::init()
{
    Board::init();
    database.init();
    sysConfig.init();

    #ifdef __AVR__
    //enable global interrupts
    sei();
    #endif

    sysConfig.configureMIDI();
    Board::ledFlashStartup(Board::checkNewRevision());

    #ifdef LEDS_SUPPORTED
    leds.init();
    #endif

    encoders.init();

    #ifdef DISPLAY_SUPPORTED
    if (database.read(DB_BLOCK_DISPLAY, dbSection_display_features, displayFeatureEnable))
    {
        if (display.init(static_cast<displayController_t>(database.read(DB_BLOCK_DISPLAY, dbSection_display_hw, displayHwController)), static_cast<displayResolution_t>(database.read(DB_BLOCK_DISPLAY, dbSection_display_hw, displayHwResolution)), false))
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

    #ifdef TOUCHSCREEN_SUPPORTED
    #ifdef BOARD_BERGAMOT
    touchscreen.init();
    touchscreen.setScreen(1);
    #endif
    #endif

    cinfo.registerHandler([](dbBlockID_t dbBlock, SysExConf::sysExParameter_t componentID)
    {
        return sysConfig.sendCInfo(dbBlock, componentID);
    });

    analog.setButtonHandler([](uint8_t analogIndex, uint16_t adcValue)
    {
        buttons.processButton(analogIndex+MAX_NUMBER_OF_BUTTONS, buttons.getStateFromAnalogValue(adcValue));
    });

    #ifdef TOUCHSCREEN_SUPPORTED
    touchscreen.setButtonHandler([](uint8_t index, bool state)
    {
        buttons.processButton(MAX_NUMBER_OF_BUTTONS+MAX_NUMBER_OF_ANALOG+index, state);
    });
    #endif

    database.setPresetChangeHandler([](uint8_t preset)
    {
        #ifdef LEDS_SUPPORTED
        leds.midiToState(MIDI::messageType_t::programChange, preset, 0, 0, true);
        #endif

        #ifdef DISPLAY_SUPPORTED
        display.init(static_cast<displayController_t>(database.read(DB_BLOCK_DISPLAY, dbSection_display_hw, displayHwController)), static_cast<displayResolution_t>(database.read(DB_BLOCK_DISPLAY, dbSection_display_hw, displayHwResolution)));
        display.setRetentionState(database.read(DB_BLOCK_DISPLAY, dbSection_display_features, displayFeatureMIDIeventRetention));
        display.setRetentionTime(database.read(DB_BLOCK_DISPLAY, dbSection_display_features, displayFeatureMIDIeventTime) * 1000);
        display.setAlternateNoteDisplay(database.read(DB_BLOCK_DISPLAY, dbSection_display_features, displayFeatureMIDInotesAlternate));
        display.displayMIDIevent(Interface::Display::eventType_t::in, Interface::Display::event_t::presetChange, preset, 0, 0);
        #endif
    });
}

void OpenDeck::checkComponents()
{
    if (sysConfig.isProcessingEnabled())
    {
        if (Board::interface::digital::input::isDataAvailable())
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
    auto processMessage = [](MIDI::interface_t interface)
    {
        //new message
        auto messageType = midi.getType(interface);
        uint8_t data1 = midi.getData1(interface);
        uint8_t data2 = midi.getData2(interface);
        uint8_t channel = midi.getChannel(interface);

        switch(messageType)
        {
            case MIDI::messageType_t::systemExclusive:
            sysConfig.handleMessage(midi.getSysExArray(interface), midi.getSysExArrayLength(interface));
            break;

            case MIDI::messageType_t::noteOn:
            case MIDI::messageType_t::noteOff:
            case MIDI::messageType_t::controlChange:
            case MIDI::messageType_t::programChange:
            #if defined(LEDS_SUPPORTED) || defined(DISPLAY_SUPPORTED)
            if (messageType == MIDI::messageType_t::noteOff)
                data2 = 0;
            #endif
            #ifdef LEDS_SUPPORTED
            leds.midiToState(messageType, data1, data2, channel);
            #endif
            #ifdef DISPLAY_SUPPORTED
            switch(messageType)
            {
                case MIDI::messageType_t::noteOn:
                display.displayMIDIevent(Interface::Display::eventType_t::in, Interface::Display::event_t::noteOn, data1, data2, channel+1);
                break;

                case MIDI::messageType_t::noteOff:
                display.displayMIDIevent(Interface::Display::eventType_t::in, Interface::Display::event_t::noteOff, data1, data2, channel+1);
                break;

                case MIDI::messageType_t::controlChange:
                display.displayMIDIevent(Interface::Display::eventType_t::in, Interface::Display::event_t::controlChange, data1, data2, channel+1);
                break;

                case MIDI::messageType_t::programChange:
                display.displayMIDIevent(Interface::Display::eventType_t::in, Interface::Display::event_t::programChange, data1, data2, channel+1);
                break;

                default:
                break;
            }
            #endif

            if (messageType == MIDI::messageType_t::programChange)
                database.setPreset(data1);

            if (messageType == MIDI::messageType_t::controlChange)
            {
                for (int i=0; i<MAX_NUMBER_OF_ENCODERS; i++)
                {
                    if (!database.read(DB_BLOCK_ENCODERS, dbSection_encoders_remoteSync, i))
                        continue;

                    if (database.read(DB_BLOCK_ENCODERS, dbSection_encoders_mode, i) != static_cast<int32_t>(Interface::digital::input::Encoders::type_t::tControlChange))
                        continue;

                    if (database.read(DB_BLOCK_ENCODERS, dbSection_encoders_midiChannel, i) != channel)
                        continue;

                    if (database.read(DB_BLOCK_ENCODERS, dbSection_encoders_midiID, i) != data1)
                        continue;

                    encoders.setValue(i, data2);
                }
            }
            break;

            case MIDI::messageType_t::sysRealTimeClock:
            #ifdef LEDS_SUPPORTED
            leds.checkBlinking(true);
            #endif
            break;

            case MIDI::messageType_t::sysRealTimeStart:
            #ifdef LEDS_SUPPORTED
            leds.resetBlinking();
            leds.checkBlinking(true);
            #endif
            break;

            default:
            break;
        }
    };

    //note: mega/uno
    //"fake" usb interface - din data is stored as usb data so use usb callback to read the usb
    //packet stored in midi object
    if (midi.read(MIDI::interface_t::usb))
        processMessage(MIDI::interface_t::usb);

    #ifdef DIN_MIDI_SUPPORTED
    if (database.read(DB_BLOCK_GLOBAL, dbSection_global_midiFeatures, midiFeatureDinEnabled))
    {
        if (database.read(DB_BLOCK_GLOBAL, dbSection_global_midiFeatures, midiFeatureMergeEnabled))
        {
            switch(database.read(DB_BLOCK_GLOBAL, dbSection_global_midiMerge, midiMergeType))
            {
                case midiMergeDINtoUSB:
                //dump everything from DIN MIDI in to USB MIDI out
                midi.read(MIDI::interface_t::din, MIDI::filterMode_t::fullUSB);
                break;

                // case midiMergeDINtoDIN:
                //loopback is automatically configured here
                // break;

                // case midiMergeODmaster:
                //already configured
                // break;

                case midiMergeODslaveInitial:
                //handle the traffic regulary until slave is properly configured
                //(upon receiving message from master)
                if (midi.read(MIDI::interface_t::din))
                    processMessage(MIDI::interface_t::din);
                break;

                default:
                break;
            }
        }
        else
        {
            if (midi.read(MIDI::interface_t::din))
                processMessage(MIDI::interface_t::din);
        }
    }
    #endif
}

void OpenDeck::update()
{
    checkMIDI();
    checkComponents();
}