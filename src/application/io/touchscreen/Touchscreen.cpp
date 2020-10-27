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

#include "Touchscreen.h"

using namespace IO;

#define MODEL modelPtr[activeModel]

core::RingBuffer<uint8_t, IO::Touchscreen::Model::Common::bufferSize> IO::Touchscreen::Model::Common::rxBuffer;

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
            //screen 0 should be blank or logo only - used to detect that the firmware is running
            setScreen(1);
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

    size_t buttonID = 0;
    bool   state    = false;

    if (MODEL->update(buttonID, state))
    {
        bool   changeScreen = false;
        size_t newScreen    = 0;

        if (database.read(Database::Section::touchscreen_t::pageSwitchEnabled, buttonID))
        {
            changeScreen = true;
            newScreen    = database.read(Database::Section::touchscreen_t::pageSwitchIndex, buttonID);
        }

        if (buttonHandler != nullptr)
            (*buttonHandler)(buttonID, state);

        //if the button should change screen, change it immediately
        //this will result in button never sending off state so do it manually first
        if (changeScreen)
        {
            if (buttonHandler != nullptr)
                (*buttonHandler)(buttonID, false);

            setScreen(newScreen);
        }
    }
}

///
/// \brief Switches to requested screen on display
/// @param [in] screenID  Index of screen to display.
///
void Touchscreen::setScreen(size_t screenID)
{
    if (!initialized)
        return;

    MODEL->setScreen(screenID);
    activeScreenID = screenID;

    if (screenHandler != nullptr)
        screenHandler(screenID);
}

///
/// \brief Used to retrieve currently active screen on display.
/// \returns Active display screen.
///
size_t Touchscreen::activeScreen()
{
    return activeScreenID;
}

void Touchscreen::setButtonHandler(void (*fptr)(size_t index, bool state))
{
    buttonHandler = fptr;
}

void Touchscreen::setScreenChangeHandler(void (*fptr)(size_t screenID))
{
    screenHandler = fptr;
}

void Touchscreen::setIconState(size_t index, bool state)
{
    if (!initialized)
        return;

    if (index >= MAX_NUMBER_OF_TOUCHSCREEN_BUTTONS)
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