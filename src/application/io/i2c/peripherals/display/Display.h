/*

Copyright 2015-2022 Igor Petrovic

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

#include "u8g2/csrc/u8x8.h"
#include "core/src/general/StringBuilder.h"
#include "database/Database.h"
#include "util/messaging/Messaging.h"
#include "system/Config.h"
#include "io/i2c/I2C.h"

namespace IO
{
    class Display : public IO::I2C::Peripheral
    {
        public:
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
            deviceInfoMsg,
            controller,
            resolution,
            MIDIeventTime,
            MIDInotesAlternate,
            octaveNormalization,
            enable,
            AMOUNT
        };

        enum class displayController_t : uint8_t
        {
            invalid,
            ssd1306,
            AMOUNT
        };

        enum displayResolution_t : uint8_t
        {
            invalid,
            _128x64,
            _128x32,
            AMOUNT
        };

        Display(I2C::HWA& hwa,
                Database& database);

        bool           init(uint8_t address) override;
        void           update() override;
        const uint8_t* addresses(size_t& amount) override;

        private:
        bool                   initU8X8(uint8_t i2cAddress, displayController_t controller, displayResolution_t resolution);
        bool                   deInit();
        void                   setAlternateNoteDisplay(bool state);
        void                   setRetentionTime(uint32_t retentionTime);
        void                   displayMIDIevent(eventType_t type, const Util::MessageDispatcher::message_t& dispatchMessage);
        void                   displayWelcomeMessage();
        void                   updateText(uint8_t row, uint8_t startIndex);
        uint8_t                getTextCenter(uint8_t textSize);
        int8_t                 normalizeOctave(uint8_t octave, int8_t normalization);
        void                   buildString(const char* text, ...);
        void                   updateScrollStatus(uint8_t row);
        void                   clearMIDIevent(eventType_t type);
        std::optional<uint8_t> sysConfigGet(System::Config::Section::i2c_t section, size_t index, uint16_t& value);
        std::optional<uint8_t> sysConfigSet(System::Config::Section::i2c_t section, size_t index, uint16_t value);

        Database&        _database;
        u8x8_t           _u8x8;
        static I2C::HWA* _hwa;

        static constexpr uint8_t _i2cAddress[2] = {
            0x3C,
            0x3D
        };

        /// Size of buffer used to build text string on display in bytes.
        static constexpr uint8_t LCD_STRING_BUFFER_SIZE = 40;

        /// Length of temporary (message) text on display in milliseconds.
        static constexpr uint16_t LCD_MESSAGE_DURATION = 1500;

        /// Time in milliseconds after text on display is being refreshed.
        static constexpr uint16_t LCD_REFRESH_TIME = 30;

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

        /// Array holding LCD text for each LCD row.
        char _lcdRowText[LCD_HEIGHT_MAX][LCD_STRING_BUFFER_SIZE] = {};

        /// Array holding true of false value representing the change of character at specific location on LCD row.
        /// \warning This variables assume there can be no more than 32 characters per LCD row.
        uint32_t _charChange[LCD_HEIGHT_MAX] = {};

        /// Structure array holding scrolling information for all LCD rows.
        scrollEvent_t _scrollEvent[LCD_HEIGHT_MAX] = {};

        /// Holds last time text has been scrolled on display.
        uint32_t _lastScrollTime = 0;

        /// Holds value by which actual octave is being subtracted when showing octave on display.
        int8_t _octaveNormalization = 0;

        /// Holds resolution of configured screen.
        displayResolution_t _resolution = displayResolution_t::AMOUNT;

        /// Holds true if display has been initialized.
        bool _initialized = false;

        /// Object used for easier string manipulation on display.
        core::StringBuilder<LCD_STRING_BUFFER_SIZE> _stringBuilder;

        /// Array holding remapped values of LCD rows.
        /// Used to increase readability.
        /// Matched with displayResolution_t enum.
        const uint8_t _rowMap[static_cast<uint8_t>(displayResolution_t::AMOUNT)][LCD_HEIGHT_MAX] = {
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

        uint8_t _activePreset       = 0;
        bool    _startupInfoShown   = false;
        uint8_t _selectedI2Caddress = 0;
        size_t  _rows               = 0;
        size_t  _columns            = 0;
    };
}    // namespace IO