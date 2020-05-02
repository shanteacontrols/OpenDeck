/*

Copyright 2015-2020 Igor Petrovic

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
#include "Config.h"
#include "core/src/general/StringBuilder.h"
#include "database/Database.h"

namespace IO
{
    ///
    /// \brief LCD control.
    /// \defgroup interfaceLCD LCD
    /// \ingroup interface
    /// @{

    class Display
    {
        public:
        ///
        /// \brief List of all possible text types on display.
        ///
        enum class lcdTextType_t : uint8_t
        {
            still,
            temp
        };

        ///
        /// \brief List of all possible text scrolling directions.
        ///
        enum class scrollDirection_t : uint8_t
        {
            leftToRight,
            rightToLeft
        };

        ///
        /// \brief Structure holding data for scrolling event on display for single row.
        ///
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

        ///
        /// \brief Same enumeration as MIDI::messageType_t but without custom values.
        /// Used for easier access to event strings stored in arrays
        ///
        enum class event_t : uint8_t
        {
            noteOff,
            noteOn,
            controlChange,
            programChange,
            afterTouchChannel,
            afterTouchPoly,
            pitchBend,
            systemExclusive,
            sysCommonTimeCodeQuarterFrame,
            sysCommonSongPosition,
            sysCommonSongSelect,
            sysCommonTuneRequest,
            sysRealTimeClock,
            sysRealTimeStart,
            sysRealTimeContinue,
            sysRealTimeStop,
            sysRealTimeActiveSensing,
            sysRealTimeSystemReset,
            //these messages aren't part of regular MIDI::messageType_t enum
            mmcPlay,
            mmcStop,
            mmcPause,
            mmcRecordOn,
            mmcRecordOff,
            nrpn,
            presetChange
        };

        enum class setting_t : uint8_t
        {
            controller,
            resolution,
            MIDIeventTime,
            octaveNormalization,
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

        Display(Database& database)
            : database(database)
        {}

        bool init(bool startupInfo);
        bool update();
        void displayMIDIevent(eventType_t type, event_t event, uint16_t byte1, uint16_t byte2, uint8_t byte3);
        void setAlternateNoteDisplay(bool state);
        void setOctaveNormalization(int8_t value);
        void setRetentionTime(uint32_t time);

        private:
        void          displayHome();
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

        Database& database;

        ///
        /// \brief Holds last time index MIDI message was shown for specific event type (in or out).
        ///
        uint32_t lastMIDIMessageDisplayTime[2] = {};

        ///
        /// \brief Holds true if MIDI input or output message is shown on display.
        ///
        bool midiMessageDisplayed[2] = {};

        ///
        /// \brief Holds time after which MIDI message should be cleared on display if retention is disabled.
        ///
        uint32_t MIDImessageRetentionTime = 0;

        ///
        /// \brief Holds last time index info message was shown.
        ///
        uint32_t messageDisplayTime = 0;

        ///
        /// \brief Holds last time LCD was updated.
        /// LCD isn't updated in real-time but after defined amount of time (see LCD_REFRESH_TIME).
        ///
        uint32_t lastLCDupdateTime = 0;

        ///
        /// \brief Holds active text type on display.
        /// Enumerated type (see lcdTextType_t enumeration).
        ///
        lcdTextType_t activeTextType = lcdTextType_t::still;

        ///
        /// \brief Array holding still LCD text for each LCD row.
        ///
        char lcdRowStillText[LCD_HEIGHT_MAX][LCD_STRING_BUFFER_SIZE] = {};

        ///
        /// \brief Array holding temp LCD text for each LCD row.
        ///
        char lcdRowTempText[LCD_HEIGHT_MAX][LCD_STRING_BUFFER_SIZE] = {};

        ///
        /// \brief Array holding true of false value representing the change of character at specific location on LCD row.
        /// \warning This variables assume there can be no more than 32 characters per LCD row.
        ///
        uint32_t charChange[LCD_HEIGHT_MAX] = {};

        ///
        /// \brief Structure array holding scrolling information for all LCD rows.
        ///
        scrollEvent_t scrollEvent[LCD_HEIGHT_MAX] = {};

        ///
        /// \brief Holds last time text has been scrolled on display.
        ///
        uint32_t lastScrollTime = 0;

        ///
        /// \brief Holds value by which actual octave is being subtracted when showing octave on display.
        ///
        int8_t octaveNormalization = 0;

        ///
        /// \brief If set to true, note number will be shown on display (0-127), otherwise, note and octave
        /// will be displayed (i.e. C#4).
        bool alternateNoteDisplay = true;

        ///
        /// \brief Holds true if direct LCD writing is enabled, false otherwise.
        ///
        bool directWriteState = false;

        ///
        /// \brief Holds resolution of configured screen.
        ///
        U8X8::displayResolution_t resolution = U8X8::displayResolution_t::AMOUNT;

        ///
        /// \brief Holds true if display has been initialized.
        ///
        bool initDone = false;

        ///
        /// \brief Object used for easier string manipulation on display.
        ///
        core::StringBuilder<LCD_STRING_BUFFER_SIZE> stringBuilder;

        ///
        /// \brief Array holding remapped values of LCD rows.
        /// Used to increase readability.
        /// Matched with displayResolution_t enum.
        ///
        const uint8_t rowMap[static_cast<uint8_t>(U8X8::displayResolution_t::AMOUNT)][LCD_HEIGHT_MAX] = {
            //128x32
            {
                0,
                2,
                3,
                4,
            },
            //128x64
            {
                0,
                2,
                4,
                6,
            }
        };

        U8X8::displayController_t lastController = U8X8::displayController_t::invalid;
        U8X8::displayResolution_t lastResolution = U8X8::displayResolution_t::invalid;
    };

    /// @}
}    // namespace IO