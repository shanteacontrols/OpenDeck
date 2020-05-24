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

#include <string.h>
#include <stdio.h>
#include "Display.h"
#include "core/src/general/Timing.h"
#include "core/src/general/Helpers.h"

using namespace IO;

///
/// \brief Initialize display driver and variables.
///
bool Display::init(bool startupInfo)
{
    if (database.read(Database::Section::display_t::features, static_cast<size_t>(feature_t::enable)))
    {
        auto    controller = static_cast<U8X8::displayController_t>(database.read(Database::Section::display_t::setting, static_cast<size_t>(setting_t::controller)));
        auto    resolution = static_cast<U8X8::displayResolution_t>(database.read(Database::Section::display_t::setting, static_cast<size_t>(setting_t::resolution)));
        uint8_t address    = database.read(Database::Section::display_t::setting, static_cast<size_t>(setting_t::i2cAddress));

        if (!startupInfo)
        {
            //avoid reinitializing display with the same settings in this case
            if (initDone)
            {
                if (lastController == controller)
                {
                    if (lastResolution == resolution)
                    {
                        if (lastAddress == address)
                        {
                            return true;
                        }
                    }
                }
            }
        }

        if (u8x8.initDisplay(address, controller, resolution))
        {
            u8x8.clearDisplay();
            u8x8.setPowerSave(0);
            u8x8.setFlipMode(0);

            this->resolution = resolution;

            u8x8.setFont(u8x8_font_pxplustandynewtv_r);
            u8x8.clearDisplay();

            //init char arrays
            for (int i = 0; i < LCD_HEIGHT_MAX; i++)
            {
                for (int j = 0; j < LCD_STRING_BUFFER_SIZE - 2; j++)
                {
                    lcdRowStillText[i][j] = ' ';
                    lcdRowTempText[i][j]  = ' ';
                }

                lcdRowStillText[i][LCD_STRING_BUFFER_SIZE - 1] = '\0';
                lcdRowTempText[i][LCD_STRING_BUFFER_SIZE - 1]  = '\0';

                scrollEvent[i].size         = 0;
                scrollEvent[i].startIndex   = 0;
                scrollEvent[i].currentIndex = 0;
                scrollEvent[i].direction    = scrollDirection_t::leftToRight;
            }

            initDone = true;

            if (startupInfo)
            {
                setDirectWriteState(true);

                if (database.read(Database::Section::display_t::features, static_cast<size_t>(feature_t::welcomeMsg)))
                    displayWelcomeMessage();

                if (database.read(Database::Section::display_t::features, static_cast<size_t>(feature_t::vInfoMsg)))
                    displayVinfo(false);

                setDirectWriteState(false);
            }

            setAlternateNoteDisplay(database.read(Database::Section::display_t::features, static_cast<size_t>(feature_t::MIDInotesAlternate)));
            setRetentionTime(database.read(Database::Section::display_t::setting, static_cast<size_t>(setting_t::MIDIeventTime)) * 1000);

            clearMIDIevent(eventType_t::in);
            clearMIDIevent(eventType_t::out);

            lastController = controller;
            lastResolution = resolution;

            return true;
        }
    }

    return false;
}

///
/// \brief Checks if LCD requires updating continuously.
///
bool Display::update()
{
    if (!initDone)
        return false;

    if ((core::timing::currentRunTimeMs() - lastLCDupdateTime) < LCD_REFRESH_TIME)
        return false;    //we don't need to update lcd in real time

    //use char pointer to point to line we're going to print
    char* charPointer;

    updateTempTextStatus();

    for (int i = 0; i < LCD_HEIGHT_MAX; i++)
    {
        if (activeTextType == lcdTextType_t::still)
        {
            //scrolling is possible only with still text
            updateScrollStatus(i);
            charPointer = lcdRowStillText[i];
        }
        else
        {
            charPointer = lcdRowTempText[i];
        }

        if (!charChange[i])
            continue;

        int8_t string_len = strlen(charPointer) > LCD_WIDTH_MAX ? LCD_WIDTH_MAX : strlen(charPointer);

        for (int j = 0; j < string_len; j++)
        {
            if (BIT_READ(charChange[i], j))
                u8x8.drawGlyph(j, rowMap[resolution][i], charPointer[j + scrollEvent[i].currentIndex]);
        }

        //now fill remaining columns with spaces
        for (int j = string_len; j < LCD_WIDTH_MAX; j++)
            u8x8.drawGlyph(j, rowMap[resolution][i], ' ');

        charChange[i] = 0;
    }

    lastLCDupdateTime = core::timing::currentRunTimeMs();

    //check if midi in/out messages need to be cleared
    if (MIDImessageRetentionTime)
    {
        for (int i = 0; i < 2; i++)
        {
            //0 = in, 1 = out
            if ((core::timing::currentRunTimeMs() - lastMIDIMessageDisplayTime[i] > MIDImessageRetentionTime) && midiMessageDisplayed[i])
                clearMIDIevent(static_cast<eventType_t>(i));
        }
    }

    return true;
}

///
/// \brief Updates text to be shown on display.
/// This function only updates internal buffers with received text, actual updating is done in update() function.
/// Text isn't passed directly, instead, value from string builder is used.
/// @param [in] row             Row which is being updated.
/// @param [in] textType        Type of text to be shown on display (enumerated type). See lcdTextType_t enumeration.
/// @param [in] startIndex      Index on which received text should on specified row.
///
void Display::updateText(uint8_t row, lcdTextType_t textType, uint8_t startIndex)
{
    if (!initDone)
        return;

    const char* string     = stringBuilder.string();
    uint8_t     size       = strlen(string);
    uint8_t     scrollSize = 0;

    if (size + startIndex >= LCD_STRING_BUFFER_SIZE - 2)
        size = LCD_STRING_BUFFER_SIZE - 2 - startIndex;    //trim string

    if (directWriteState)
    {
        for (int j = 0; j < size; j++)
            u8x8.drawGlyph(j + startIndex, rowMap[resolution][row], string[j]);
    }
    else
    {
        bool scrollingEnabled = false;

        switch (textType)
        {
        case lcdTextType_t::still:
            for (int i = 0; i < size; i++)
            {
                lcdRowStillText[row][startIndex + i] = string[i];
                BIT_WRITE(charChange[row], startIndex + i, 1);
            }

            //scrolling is enabled only if some characters are found after LCD_WIDTH_MAX-1 index
            for (int i = LCD_WIDTH_MAX; i < LCD_STRING_BUFFER_SIZE - 1; i++)
            {
                if ((lcdRowStillText[row][i] != ' ') && (lcdRowStillText[row][i] != '\0'))
                {
                    scrollingEnabled = true;
                    scrollSize++;
                }
            }

            if (scrollingEnabled && !scrollEvent[row].size)
            {
                //enable scrolling
                scrollEvent[row].size         = scrollSize;
                scrollEvent[row].startIndex   = startIndex;
                scrollEvent[row].currentIndex = 0;
                scrollEvent[row].direction    = scrollDirection_t::leftToRight;

                lastScrollTime = core::timing::currentRunTimeMs();
            }
            else if (!scrollingEnabled && scrollEvent[row].size)
            {
                scrollEvent[row].size         = 0;
                scrollEvent[row].startIndex   = 0;
                scrollEvent[row].currentIndex = 0;
                scrollEvent[row].direction    = scrollDirection_t::leftToRight;
            }
            break;

        case lcdTextType_t::temp:
            //clear entire message first
            for (int j = 0; j < LCD_WIDTH_MAX - 2; j++)
                lcdRowTempText[row][j] = ' ';

            lcdRowTempText[row][LCD_WIDTH_MAX - 1] = '\0';

            for (int i = 0; i < size; i++)
                lcdRowTempText[row][startIndex + i] = string[i];

            //make sure message is properly EOL'ed
            lcdRowTempText[row][startIndex + size] = '\0';

            activeTextType     = lcdTextType_t::temp;
            messageDisplayTime = core::timing::currentRunTimeMs();

            //update all characters on display
            for (int i = 0; i < LCD_HEIGHT_MAX; i++)
                charChange[i] = static_cast<uint32_t>(0xFFFFFFFF);
            break;

        default:
            return;
        }
    }
}

///
/// \brief Enables or disables direct writing to LCD.
/// When enabled, low-level APIs are used to write text to LCD directly.
/// Otherwise, update() function takes care of updating LCD.
/// @param [in] state   New direct write state.
///
void Display::setDirectWriteState(bool state)
{
    directWriteState = state;
}

///
/// \brief Calculates position on which text needs to be set on display to be in center of display row.
/// @param [in] textSize    Size of text for which center position on display is being calculated.
/// \returns Center position of text on display.
///
uint8_t Display::getTextCenter(uint8_t textSize)
{
    return u8x8.getColumns() / 2 - (textSize / 2);
}

///
/// \brief Updates status of temp text on display.
///
void Display::updateTempTextStatus()
{
    if (activeTextType == lcdTextType_t::temp)
    {
        //temp text - check if temp text should be removed
        if ((core::timing::currentRunTimeMs() - messageDisplayTime) > LCD_MESSAGE_DURATION)
        {
            activeTextType = lcdTextType_t::still;

            //make sure all characters are updated once temp text is removed
            for (int j = 0; j < LCD_HEIGHT_MAX; j++)
                charChange[j] = static_cast<uint32_t>(0xFFFFFFFF);
        }
    }
}

///
/// \brief Updates status of scrolling text on display.
/// @param [in] row     Row which is being checked.
///
void Display::updateScrollStatus(uint8_t row)
{
    if (!scrollEvent[row].size)
        return;

    if ((core::timing::currentRunTimeMs() - lastScrollTime) < LCD_SCROLL_TIME)
        return;

    switch (scrollEvent[row].direction)
    {
    case scrollDirection_t::leftToRight:
        //left to right
        scrollEvent[row].currentIndex++;

        if (scrollEvent[row].currentIndex == scrollEvent[row].size)
        {
            //switch direction
            scrollEvent[row].direction = scrollDirection_t::rightToLeft;
        }
        break;

    case scrollDirection_t::rightToLeft:
        //right to left
        scrollEvent[row].currentIndex--;

        if (scrollEvent[row].currentIndex == 0)
        {
            //switch direction
            scrollEvent[row].direction = scrollDirection_t::leftToRight;
        }
        break;
    }

    for (int i = scrollEvent[row].startIndex; i < LCD_WIDTH_MAX; i++)
        BIT_WRITE(charChange[row], i, 1);

    lastScrollTime = core::timing::currentRunTimeMs();
}

///
/// \brief Checks for currently active text type on display.
/// \returns Active text type (enumerated type). See lcdTextType_t enumeration.
///
Display::lcdTextType_t Display::getActiveTextType()
{
    return activeTextType;
}

///
/// \brief Sets new message retention time.
/// @param [in] retentionTime New retention time in milliseconds.
///
void Display::setRetentionTime(uint32_t retentionTime)
{
    if (retentionTime < MIDImessageRetentionTime)
    {
        for (int i = 0; i < 2; i++)
        {
            //0 = in, 1 = out
            //make sure events are cleared immediately in next call of update()
            lastMIDIMessageDisplayTime[i] = 0;
        }
    }

    MIDImessageRetentionTime = retentionTime;

    //reset last update time
    lastMIDIMessageDisplayTime[eventType_t::in]  = core::timing::currentRunTimeMs();
    lastMIDIMessageDisplayTime[eventType_t::out] = core::timing::currentRunTimeMs();
}

///
/// \brief Adds normalization to a given octave.
///
int8_t Display::normalizeOctave(uint8_t octave, int8_t normalization)
{
    return static_cast<int8_t>(octave) + normalization;
}