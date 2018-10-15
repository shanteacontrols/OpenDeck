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

#pragma once

#include "sysex/src/SysEx.h"
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

class SysConfig : public SysEx
{
    public:
    #ifdef LEDS_SUPPORTED
    #ifdef DISPLAY_SUPPORTED
    SysConfig(Database &database, MIDI &midi, Buttons &buttons, Encoders &encoders, Analog &analog, LEDs &leds, Display &display) :
    #else
    SysConfig(Database &database, MIDI &midi, Buttons &buttons, Encoders &encoders, Analog &analog, LEDs &leds) :
    #endif
    #else
    #ifdef DISPLAY_SUPPORTED
    SysConfig(Database &database, MIDI &midi, Buttons &buttons, Encoders &encoders, Analog &analog, Display &display) :
    #else
    SysConfig(Database &database, MIDI &midi, Buttons &buttons, Encoders &encoders, Analog &analog) :
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

    bool sendCInfo(dbBlockID_t dbBlock, sysExParameter_t componentID);

    private:
    Database    &database;
    MIDI        &midi;
    Buttons     &buttons;
    Encoders    &encoders;
    Analog      &analog;
    #ifdef LEDS_SUPPORTED
    LEDs        &leds;
    #endif
    #ifdef DISPLAY_SUPPORTED
    Display     &display;
    #endif

    ///
    /// Used to prevent updating states of all components (analog, LEDs, encoders, buttons).
    ///
    bool     processingEnabled = true;

    bool onGet(uint8_t block, uint8_t section, uint16_t index, sysExParameter_t &value);
    bool onSet(uint8_t block, uint8_t section, uint16_t index, sysExParameter_t newValue);
    bool onCustomRequest(uint8_t value);
    void onWrite(uint8_t *sysExArray, uint8_t size);

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