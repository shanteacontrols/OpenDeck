/*

Copyright 2015-2021 Igor Petrovic

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

#include "U8X8/U8X8.h"
#include "core/src/general/StringBuilder.h"
#include "database/Database.h"
#include "util/messaging/Messaging.h"

#ifndef DISPLAY_SUPPORTED
#include "stub/Display.h"
#else

namespace IO
{
    class Display
    {
        public:
        /// List of all possible text types on display.
        enum class lcdTextType_t : uint8_t
        {
            still,
            temp
        };

        /// List of all possible text scrolling directions.
        enum class scrollDirection_t : uint8_t
        {
            leftToRight,
            rightToLeft
        };

        /// Structure holding data for scrolling event on display for single row.
        typedef struct
        {
            uint8_t           size;
            uint8_t           startIndex;
            int8_t            currentIndex;
            scrollDirection_t direction;
        } scrollEvent_t;

        enum eventType_t : uint8_t
        {
            in,
            out,
        };

        enum class setting_t : uint8_t
        {
            controller,
            resolution,
            MIDIeventTime,
            octaveNormalization,
            i2cAddress,
            AMOUNT
        };

        enum class feature_t : uint8_t
        {
            enable,
            welcomeMsg,
            vInfoMsg,
            MIDInotesAlternate,
            AMOUNT
        };

        Display(IO::U8X8&                u8x8,
                Database&                database,
                Util::MessageDispatcher& dispatcher);

        bool init(bool startupInfo);
        bool deInit();
        bool update();
        void setAlternateNoteDisplay(bool state);
        void setOctaveNormalization(int8_t value);
        void setRetentionTime(uint32_t time);
        void setPreset(uint8_t preset);

        private:
        void          displayMIDIevent(eventType_t type, const Util::MessageDispatcher::message_t& dispatchMessage);
        void          displayWelcomeMessage();
        void          displayVinfo(bool newFw);
        void          setDirectWriteState(bool state);
        lcdTextType_t getActiveTextType();
        void          updateText(uint8_t row, lcdTextType_t textType, uint8_t startIndex);
        uint8_t       getTextCenter(uint8_t textSize);
        int8_t        normalizeOctave(uint8_t octave, int8_t normalization);
        void          buildString(const char* text, ...);
        void          updateScrollStatus(uint8_t row);
        void          updateTempTextStatus();
        void          clearMIDIevent(eventType_t type);

        IO::U8X8& _u8x8;
        Database& _database;

        /// Size of buffer used to build text string on display in bytes.
        static constexpr uint8_t LCD_STRING_BUFFER_SIZE = 40;

        /// Length of temporary (message) text on display in milliseconds.
        static constexpr uint16_t LCD_MESSAGE_DURATION = 1500;

        /// Time in milliseconds after text on display is being refreshed.
        static constexpr uint16_t LCD_REFRESH_TIME = 10;

        /// Time in milliseconds after which scrolling text moves on display.
        static constexpr uint16_t LCD_SCROLL_TIME = 1000;

        /// Maximum amount of characters displayed in single LCD row.
        /// Real width is determined later based on display type.
        static constexpr uint16_t LCD_WIDTH_MAX = 32;

        /// Maximum number of LCD rows.
        /// Real height is determined later based on display type.
        static constexpr uint8_t LCD_HEIGHT_MAX = 4;

        static constexpr uint8_t COLUMN_START_MIDI_IN_MESSAGE  = 4;
        static constexpr uint8_t COLUMN_START_MIDI_OUT_MESSAGE = 5;

        static constexpr uint8_t ROW_START_MIDI_IN_MESSAGE  = 0;
        static constexpr uint8_t ROW_START_MIDI_OUT_MESSAGE = 2;

        /// Holds last time index MIDI message was shown for specific event type (in or out).
        uint32_t _lastMIDIMessageDisplayTime[2] = {};

        /// Holds true if MIDI input or output message is shown on display.
        bool _midiMessageDisplayed[2] = {};

        /// Holds time after which MIDI message should be cleared on display if retention is disabled.
        uint32_t _MIDImessageRetentionTime = 0;

        /// Holds last time index info message was shown.
        uint32_t _messageDisplayTime = 0;

        /// Holds last time LCD was updated.
        /// LCD isn't updated in real-time but after defined amount of time (see LCD_REFRESH_TIME).
        uint32_t _lastLCDupdateTime = 0;

        /// Holds active text type on display.
        /// Enumerated type (see lcdTextType_t enumeration).
        lcdTextType_t _activeTextType = lcdTextType_t::still;

        /// Array holding still LCD text for each LCD row.
        char _lcdRowStillText[LCD_HEIGHT_MAX][LCD_STRING_BUFFER_SIZE] = {};

        /// Array holding temp LCD text for each LCD row.
        char _lcdRowTempText[LCD_HEIGHT_MAX][LCD_STRING_BUFFER_SIZE] = {};

        /// Array holding true of false value representing the change of character at specific location on LCD row.
        /// \warning This variables assume there can be no more than 32 characters per LCD row.
        uint32_t _charChange[LCD_HEIGHT_MAX] = {};

        /// Structure array holding scrolling information for all LCD rows.
        scrollEvent_t _scrollEvent[LCD_HEIGHT_MAX] = {};

        /// Holds last time text has been scrolled on display.
        uint32_t _lastScrollTime = 0;

        /// Holds value by which actual octave is being subtracted when showing octave on display.
        int8_t _octaveNormalization = 0;

        /// If set to true, note number will be shown on display (0-127), otherwise, note and octave
        /// will be displayed (i.e. C#4).
        bool _alternateNoteDisplay = true;

        /// Holds true if direct LCD writing is enabled, false otherwise.
        bool _directWriteState = false;

        /// Holds resolution of configured screen.
        U8X8::displayResolution_t _resolution = U8X8::displayResolution_t::AMOUNT;

        /// Holds true if display has been initialized.
        bool _initialized = false;

        /// Object used for easier string manipulation on display.
        core::StringBuilder<LCD_STRING_BUFFER_SIZE> _stringBuilder;

        /// Array holding remapped values of LCD rows.
        /// Used to increase readability.
        /// Matched with displayResolution_t enum.
        const uint8_t _rowMap[static_cast<uint8_t>(U8X8::displayResolution_t::AMOUNT)][LCD_HEIGHT_MAX] = {
            // 128x32
            {
                0,
                2,
                3,
                4,
            },
            // 128x64
            {
                0,
                2,
                4,
                6,
            }
        };

        U8X8::displayController_t _lastController = U8X8::displayController_t::invalid;
        U8X8::displayResolution_t _lastResolution = U8X8::displayResolution_t::invalid;
        uint8_t                   _lastAddress    = 0;
        uint8_t                   _activePreset   = 0;
    };
}    // namespace IO

#endif