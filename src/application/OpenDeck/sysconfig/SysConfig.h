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

#pragma once

#include "sysex/src/SysExConf.h"
#include "CustomIDs.h"
#include "blocks/Blocks.h"
#include "board/Board.h"
#include "database/Database.h"
#include "midi/src/MIDI.h"
#include "interface/digital/input/buttons/Buttons.h"
#include "interface/digital/input/encoders/Encoders.h"
#include "interface/analog/Analog.h"
#ifdef LEDS_SUPPORTED
#include "interface/digital/output/leds/LEDs.h"
#endif

class SysConfig : public SysExConf
{
    public:
    #ifdef LEDS_SUPPORTED
    #ifdef DISPLAY_SUPPORTED
    SysConfig(Database &database, MIDI &midi, Interface::digital::input::Buttons &buttons, Interface::digital::input::Encoders &encoders, Interface::analog::Analog &analog, Interface::digital::output::LEDs &leds, Interface::Display &display) :
    #else
    SysConfig(Database &database, MIDI &midi, Interface::digital::input::Buttons &buttons, Interface::digital::input::Encoders &encoders, Interface::analog::Analog &analog, Interface::digital::output::LEDs &leds) :
    #endif
    #else
    #ifdef DISPLAY_SUPPORTED
    SysConfig(Database &database, MIDI &midi, Interface::digital::input::Buttons &buttons, Interface::digital::input::Encoders &encoders, Interface::analog::Analog &analog, Display &display) :
    #else
    SysConfig(Database &database, MIDI &midi, Interface::digital::input::Buttons &buttons, Interface::digital::input::Encoders &encoders, Interface::analog::Analog &analog) :
    #endif
    #endif
    database(database),
    midi(midi),
    buttons(buttons),
    encoders(encoders),
    analog(analog)
    #ifdef LEDS_SUPPORTED
    ,leds(leds)
    #endif
    #ifdef DISPLAY_SUPPORTED
    ,display(display)
    #endif
    {}

    void init();
    bool isProcessingEnabled();
    void configureMIDI();

    bool sendCInfo(dbBlockID_t dbBlock, SysExConf::sysExParameter_t componentID);

    private:
    Database &database;
    MIDI &midi;
    Interface::digital::input::Buttons &buttons;
    Interface::digital::input::Encoders &encoders;
    Interface::analog::Analog &analog;
    #ifdef LEDS_SUPPORTED
    Interface::digital::output::LEDs &leds;
    #endif
    #ifdef DISPLAY_SUPPORTED
    Interface::Display &display;
    #endif

    ///
    /// Used to prevent updating states of all components (analog, LEDs, encoders, buttons).
    ///
    bool     processingEnabled = true;

    bool onGet(uint8_t block, uint8_t section, uint16_t index, SysExConf::sysExParameter_t &value) override;
    bool onSet(uint8_t block, uint8_t section, uint16_t index, SysExConf::sysExParameter_t newValue) override;
    bool onCustomRequest(uint8_t value) override;
    void onWrite(uint8_t *sysExArray, uint8_t size) override;

    ///
    /// \brief Configures UART read/write handlers for MIDI module.
    ///
    void setupMIDIoverUART(uint32_t baudRate, bool initRX, bool initTX);

    ///
    /// \brief Configures USB read/write handlers for MIDI module.
    ///
    void setupMIDIoverUSB();

    #ifdef DIN_MIDI_SUPPORTED
    void configureMIDImerge(midiMergeType_t mergeType);
    void sendDaisyChainRequest();
    #endif

    uint32_t lastCinfoMsgTime[DB_BLOCKS];
};