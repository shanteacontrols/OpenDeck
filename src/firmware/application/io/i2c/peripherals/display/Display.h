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

#include <u8x8.h>
#include "core/util/StringBuilder.h"
#include "application/database/Database.h"
#include "application/messaging/Messaging.h"
#include "application/system/Config.h"
#include "application/io/i2c/I2C.h"
#include <bits/char_traits.h>
#include "strings/Strings.h"

namespace io
{
    class Display : public io::I2C::Peripheral
    {
        public:
        enum eventType_t : uint8_t
        {
            IN,
            OUT,
        };

        enum class setting_t : uint8_t
        {
            DEVICE_INFO_MSG,
            CONTROLLER,
            RESOLUTION,
            EVENT_TIME,
            MIDI_NOTES_ALTERNATE,
            OCTAVE_NORMALIZATION,
            ENABLE,
            AMOUNT
        };

        enum class displayController_t : uint8_t
        {
            INVALID,
            SSD1306,
            AMOUNT
        };

        enum displayResolution_t : uint8_t
        {
            INVALID,
            R128X64,
            R128X32,
            AMOUNT
        };

        using Database = database::User<database::Config::Section::i2c_t>;

        Display(I2C::Peripheral::HWA& hwa,
                Database&             database);

        bool init() override;
        void update() override;

        private:
        bool                   initU8X8(uint8_t i2cAddress, displayController_t controller, displayResolution_t resolution);
        bool                   deInit();
        void                   setAlternateNoteDisplay(bool state);
        void                   setRetentionTime(uint32_t retentionTime);
        void                   displayPreset(int preset);
        void                   displayEvent(eventType_t type, const messaging::event_t& event);
        void                   displayWelcomeMessage();
        void                   updateText(uint8_t row);
        uint8_t                getTextCenter(uint8_t textSize);
        int8_t                 normalizeOctave(uint8_t octave, int8_t normalization);
        void                   buildString(const char* text, ...);
        void                   clearEvent(eventType_t type);
        std::optional<uint8_t> sysConfigGet(sys::Config::Section::i2c_t section, size_t index, uint16_t& value);
        std::optional<uint8_t> sysConfigSet(sys::Config::Section::i2c_t section, size_t index, uint16_t value);

        I2C::Peripheral::HWA& _hwa;
        Database&             _database;
        u8x8_t                _u8x8;

        static constexpr std::array<uint8_t, 2> I2C_ADDRESS = {
            0x3C,
            0x3D
        };

        /// Length of temporary (message) text on display in milliseconds.
        static constexpr uint16_t LCD_MESSAGE_DURATION = 1500;

        /// Time in milliseconds after text on display is being refreshed.
        static constexpr uint16_t LCD_REFRESH_TIME = 30;

        static constexpr uint8_t MAX_ROWS                 = 4;
        static constexpr uint8_t MAX_COLUMNS              = 16;
        static constexpr uint8_t TEXT_BUFFER_SIZE         = MAX_COLUMNS + 1;
        static constexpr auto    COLUMN_START_IN_MESSAGE  = std::char_traits<char>::length(Strings::IN_EVENT_STRING);
        static constexpr auto    COLUMN_START_OUT_MESSAGE = std::char_traits<char>::length(Strings::OUT_EVENT_STRING);
        static constexpr auto    COLUMN_START_PRESET      = 12;
        static constexpr auto    MAX_COLUMNS_IN_MESSAGE   = COLUMN_START_PRESET;
        static constexpr auto    MAX_COLUMNS_OUT_MESSAGE  = COLUMN_START_PRESET;
        static constexpr uint8_t ROW_START_IN_MESSAGE     = 0;
        static constexpr uint8_t ROW_START_OUT_MESSAGE    = 2;
        static constexpr uint8_t ROW_START_PRESET         = ROW_START_IN_MESSAGE;
        static constexpr size_t  U8X8_BUFFER_SIZE         = 32;

        uint8_t _u8x8Buffer[U8X8_BUFFER_SIZE] = {};
        size_t  _u8x8Counter                  = 0;

        /// Holds last time index message was shown for specific event type (in or out).
        uint32_t _lasMessageDisplayTime[2] = {};

        /// Holds true if input or output message is shown on display.
        bool _messageDisplayed[2] = {};

        /// Holds time after which message should be cleared on display if retention is disabled.
        uint32_t _messageRetentionTime = 0;

        /// Holds last time index info message was shown.
        uint32_t _messageDisplayTime = 0;

        /// Holds last time LCD was updated.
        /// LCD isn't updated in real-time but after defined amount of time (see LCD_REFRESH_TIME).
        uint32_t _lastLCDupdateTime = 0;

        /// Array holding LCD text for each LCD row.
        using textArray_t       = std::array<std::array<char, TEXT_BUFFER_SIZE>, MAX_ROWS>;
        textArray_t _lcdRowText = {};

        /// Array holding true of false value representing the change of character at specific location on LCD row.
        /// \warning This variables assume there can be no more than 32 characters per LCD row.
        uint32_t _charChange[MAX_ROWS] = {};

        /// Holds value by which actual octave is being subtracted when showing octave on display.
        int8_t _octaveNormalization = 0;

        /// Holds resolution of configured screen.
        displayResolution_t _resolution = displayResolution_t::AMOUNT;

        /// Holds true if display has been initialized.
        bool _initialized = false;

        /// Object used for easier string manipulation on display.
        core::util::StringBuilder<TEXT_BUFFER_SIZE> _stringBuilder;

        /// Array holding remapped values of LCD rows.
        /// Used to increase readability.
        /// Matched with displayResolution_t enum.
        static constexpr uint8_t ROW_MAP[static_cast<uint8_t>(displayResolution_t::AMOUNT)][MAX_ROWS] = {
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
    };
}    // namespace io