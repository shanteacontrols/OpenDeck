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

#include "Touchscreen.h"
#include "core/src/general/Helpers.h"

using namespace IO;

#define MODEL modelPtr[activeModel]

uint8_t IO::Touchscreen::Model::Common::rxBuffer[IO::Touchscreen::Model::Common::bufferSize];
size_t  IO::Touchscreen::Model::Common::bufferCount;

bool Touchscreen::init()
{
    if (database.read(Database::Section::touchscreen_t::setting, static_cast<size_t>(IO::Touchscreen::setting_t::enable)))
    {
        auto dbModel = static_cast<IO::Touchscreen::Model::model_t>(database.read(Database::Section::touchscreen_t::setting, static_cast<size_t>(IO::Touchscreen::setting_t::model)));

        if (initialized)
        {
            if (static_cast<uint8_t>(dbModel) == activeModel)
                return true;    //nothing to do, same model already initialized

            if (!deInit())
                return false;
        }

        if (!isModelValid(dbModel))
            return false;

        activeModel = static_cast<uint8_t>(dbModel);
        initialized = MODEL->init();

        if (initialized)
        {
            setScreen(database.read(Database::Section::touchscreen_t::setting, static_cast<size_t>(IO::Touchscreen::setting_t::initialScreen)));
            setBrightness(static_cast<brightness_t>(database.read(Database::Section::touchscreen_t::setting, static_cast<size_t>(IO::Touchscreen::setting_t::brightness))));

            return true;
        }
    }

    return false;
}

bool Touchscreen::deInit()
{
    if (!initialized)
        return false;    //nothing to do

    if (MODEL->deInit())
    {
        initialized = false;
        activeModel = static_cast<uint8_t>(Model::model_t::AMOUNT);
        return true;
    }

    return false;
}

void Touchscreen::update()
{
    if (!initialized)
        return;

    tsData_t  tsData;
    tsEvent_t event = MODEL->update(tsData);

    switch (event)
    {
    case tsEvent_t::button:
        processButton(tsData.buttonID, tsData.buttonState);
        break;

    case tsEvent_t::coordinate:
        processCoordinate(tsData.pressType, tsData.xPos, tsData.yPos);
        break;

    default:
        break;
    }
}

/// Switches to requested screen on display
/// param [in]: screenID  Index of screen to display.
void Touchscreen::setScreen(size_t screenID)
{
    if (!initialized)
        return;

    MODEL->setScreen(screenID);
    activeScreenID = screenID;

    if (screenHandler != nullptr)
        screenHandler(screenID);
}

/// Used to retrieve currently active screen on display.
/// returns: Active display screen.
size_t Touchscreen::activeScreen()
{
    return activeScreenID;
}

void Touchscreen::setScreenChangeHandler(void (*fptr)(size_t screenID))
{
    screenHandler = fptr;
}

void Touchscreen::setIconState(size_t index, bool state)
{
    if (!initialized)
        return;

    if (index >= MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS)
        return;

    icon_t icon;

    icon.onScreen  = database.read(Database::Section::touchscreen_t::onScreen, index);
    icon.offScreen = database.read(Database::Section::touchscreen_t::offScreen, index);

    if (icon.onScreen == icon.offScreen)
        return;    //invalid screen indexes

    if ((activeScreenID != icon.onScreen) && (activeScreenID != icon.offScreen))
        return;    //don't allow setting icon on wrong screen

    icon.xPos   = database.read(Database::Section::touchscreen_t::xPos, index);
    icon.yPos   = database.read(Database::Section::touchscreen_t::yPos, index);
    icon.width  = database.read(Database::Section::touchscreen_t::width, index);
    icon.height = database.read(Database::Section::touchscreen_t::height, index);

    MODEL->setIconState(icon, state);
}

bool Touchscreen::isModelValid(Model::model_t model)
{
    if (model == IO::Touchscreen::Model::model_t::AMOUNT)
        return false;

    if (modelPtr[static_cast<uint8_t>(model)] == nullptr)
        return false;

    return true;
}

bool Touchscreen::registerModel(IO::Touchscreen::Model::model_t model, Model* ptr)
{
    if (model == IO::Touchscreen::Model::model_t::AMOUNT)
        return false;

    modelPtr[static_cast<uint8_t>(model)] = ptr;
    return true;
}

void Touchscreen::processButton(const size_t buttonID, const bool state)
{
    bool   changeScreen = false;
    size_t newScreen    = 0;

    if (database.read(Database::Section::touchscreen_t::pageSwitchEnabled, buttonID))
    {
        changeScreen = true;
        newScreen    = database.read(Database::Section::touchscreen_t::pageSwitchIndex, buttonID);
    }

    buttons.processButton(MAX_NUMBER_OF_BUTTONS + MAX_NUMBER_OF_ANALOG + buttonID, state);
    cInfo.send(Database::block_t::touchscreen, buttonID);

    //if the button should change screen, change it immediately
    //this will result in button never sending off state so do it manually first
    if (changeScreen)
    {
        buttons.processButton(MAX_NUMBER_OF_BUTTONS + MAX_NUMBER_OF_ANALOG + buttonID, false);
        setScreen(newScreen);
    }
}

bool Touchscreen::setBrightness(brightness_t brightness)
{
    if (!initialized)
        return false;

    return MODEL->setBrightness(brightness);
}

void Touchscreen::processCoordinate(pressType_t pressType, uint16_t xPos, uint16_t yPos)
{
    for (size_t i = 0; i < MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS; i++)
    {
        if (database.read(Database::Section::touchscreen_t::analogPage, i) == static_cast<int32_t>(activeScreen()))
        {
            uint16_t startXCoordinate = database.read(Database::Section::touchscreen_t::analogStartXCoordinate, i);
            uint16_t endXCoordinate   = database.read(Database::Section::touchscreen_t::analogEndXCoordinate, i);
            uint16_t startYCoordinate = database.read(Database::Section::touchscreen_t::analogStartYCoordinate, i);
            uint16_t endYCoordinate   = database.read(Database::Section::touchscreen_t::analogEndYCoordinate, i);

            uint16_t startCoordinate;
            uint16_t endCoordinate;
            uint16_t value;

            //x
            if (database.read(Database::Section::touchscreen_t::analogType, i) == static_cast<int32_t>(IO::Touchscreen::analogType_t::horizontal))
            {
                value = xPos;

                if (pressType == IO::Touchscreen::pressType_t::hold)
                {
                    //y coordinate can be ignored once the touchscreen is pressed, verify and constrain x range only
                    if (value > endXCoordinate)
                        value = endXCoordinate;
                    else if (value < startXCoordinate)
                        value = startXCoordinate;
                }
                else if (pressType == IO::Touchscreen::pressType_t::initial)
                {
                    if (((value < startXCoordinate) || (value > endXCoordinate)) || ((yPos < startYCoordinate) || (yPos > endYCoordinate)))
                        continue;
                    else
                        analogActive[i] = true;
                }
                else
                {
                    analogActive[i] = false;
                }

                startCoordinate = startXCoordinate;
                endCoordinate   = endXCoordinate;
            }
            //y
            else
            {
                value = yPos;

                if (pressType == IO::Touchscreen::pressType_t::hold)
                {
                    //x coordinate can be ignored once the touchscreen is pressed, verify and constrain x range only
                    if (value > endYCoordinate)
                        value = endYCoordinate;
                    else if (value < startYCoordinate)
                        value = startYCoordinate;
                }
                else if (pressType == IO::Touchscreen::pressType_t::initial)
                {
                    if (((xPos < startXCoordinate) || (xPos > endXCoordinate)) || ((value < startYCoordinate) || (value > endYCoordinate)))
                        continue;
                    else
                        analogActive[i] = true;
                }
                else
                {
                    analogActive[i] = false;
                }

                startCoordinate = startYCoordinate;
                endCoordinate   = endYCoordinate;
            }

            //scale the value to ADC range
            if (analogActive[i])
            {
                value = core::misc::mapRange(static_cast<uint32_t>(value), static_cast<uint32_t>(startCoordinate), static_cast<uint32_t>(endCoordinate), static_cast<uint32_t>(0), static_cast<uint32_t>(analog.adcType()));
                analog.processReading(MAX_NUMBER_OF_ANALOG + i, value);
            }
            else
            {
                if (database.read(Database::Section::touchscreen_t::analogResetOnRelease, i))
                    analog.processReading(MAX_NUMBER_OF_ANALOG + i, 0);
            }
        }
    }
}