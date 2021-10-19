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

using namespace IO;

uint8_t IO::TouchscreenBase::Common::rxBuffer[IO::TouchscreenBase::Common::bufferSize];
size_t  IO::TouchscreenBase::Common::bufferCount;

Touchscreen::Touchscreen(TouchscreenBase::HWA& hwa,
                         Database&             database,
                         CDCPassthrough&       cdcPassthrough)
    : _hwa(hwa)
    , _database(database)
    , _cdcPassthrough(cdcPassthrough)
{
}

bool Touchscreen::init(mode_t mode)
{
    switch (mode)
    {
    case mode_t::normal:
    {
        if (_database.read(Database::Section::touchscreen_t::setting, IO::Touchscreen::setting_t::enable))
        {
            auto dbModel = _database.read(Database::Section::touchscreen_t::setting, IO::Touchscreen::setting_t::model);

            if (_initialized)
            {
                if (dbModel == static_cast<uint8_t>(_activeModel))
                    return true;    // nothing to do, same model already _initialized

                if (!deInit(mode))
                    return false;
            }

            if (_cdcPassthrough.deInit())
            {
                _activeModel = static_cast<model_t>(dbModel);
                _initialized = modelInstance().init();

                if (_initialized)
                {
                    setScreen(_database.read(Database::Section::touchscreen_t::setting, IO::Touchscreen::setting_t::initialScreen));
                    setBrightness(static_cast<brightness_t>(_database.read(Database::Section::touchscreen_t::setting, IO::Touchscreen::setting_t::brightness)));

                    _mode = mode;

                    return true;
                }
            }
        }

        return false;
    }
    break;

    case mode_t::cdcPassthrough:
    {
        if (_cdcPassthrough.init())
        {
            _mode = mode;
            return true;
        }
    }
    break;

    default:
        return false;
    }

    return false;
}

bool Touchscreen::deInit(mode_t mode)
{
    switch (mode)
    {
    case mode_t::normal:
    {
        if (!_initialized)
            return false;

        if (_cdcPassthrough.deInit())
        {
            if (modelInstance().deInit())
            {
                _mode        = mode_t::normal;
                _initialized = false;

                return true;
            }
        }
    }
    break;

    case mode_t::cdcPassthrough:
    {
        if (_cdcPassthrough.deInit())
        {
            _mode = mode_t::normal;
            return true;
        }
    }
    break;

    default:
        return false;
    }

    return false;
}

void Touchscreen::update()
{
    if (isInitialized(mode_t::normal))
    {
        tsData_t  tsData;
        tsEvent_t event = modelInstance().update(tsData);

        switch (event)
        {
        case tsEvent_t::button:
        {
            processButton(tsData.buttonID, tsData.buttonState);
        }
        break;

        case tsEvent_t::coordinate:
        {
            processCoordinate(tsData.pressType, tsData.xPos, tsData.yPos);
        }
        break;

        default:
            break;
        }
    }
    else if (isInitialized(mode_t::cdcPassthrough))
    {
        auto touchscreenToUSB = [&]() {
            uint32_t size = 0;

            for (size = 0; size < TSCREEN_CDC_PASSTHROUGH_BUFFER_SIZE; size++)
            {
                uint8_t value;

                if (_cdcPassthrough.uartRead(value))
                {
                    _txBuffer[size] = value;
                }
                else
                {
                    break;
                }
            }

            if (size)
            {
                _cdcPassthrough.cdcWrite(_txBuffer, size);
            }
        };

        auto usbToTouchscreen = [&]() {
            size_t size = 0;

            while (_cdcPassthrough.cdcRead(_rxBuffer, size, TSCREEN_CDC_PASSTHROUGH_BUFFER_SIZE))
            {
                for (size_t i = 0; i < size; i++)
                {
                    _cdcPassthrough.uartWrite(_rxBuffer[i]);
                }
            }
        };

        touchscreenToUSB();
        usbToTouchscreen();
    }
}

/// Switches to requested screen on display
/// param [in]: screenID  Index of screen to display.
void Touchscreen::setScreen(size_t screenID)
{
    if (!isInitialized(mode_t::normal))
        return;

    modelInstance().setScreen(screenID);
    _activeScreenID = screenID;

    if (_eventNotifier != nullptr)
        _eventNotifier->screenChange(screenID);
}

/// Used to retrieve currently active screen on display.
/// returns: Active display screen.
size_t Touchscreen::activeScreen()
{
    return _activeScreenID;
}

void Touchscreen::registerEventNotifier(EventNotifier& eventNotifer)
{
    _eventNotifier = &eventNotifer;
}

void Touchscreen::setIconState(size_t index, bool state)
{
    if (!isInitialized(mode_t::normal))
        return;

    if (index >= MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS)
        return;

    icon_t icon;

    icon.onScreen  = _database.read(Database::Section::touchscreen_t::onScreen, index);
    icon.offScreen = _database.read(Database::Section::touchscreen_t::offScreen, index);

    if (icon.onScreen == icon.offScreen)
        return;    // invalid screen indexes

    if ((_activeScreenID != icon.onScreen) && (_activeScreenID != icon.offScreen))
        return;    // don't allow setting icon on wrong screen

    icon.xPos   = _database.read(Database::Section::touchscreen_t::xPos, index);
    icon.yPos   = _database.read(Database::Section::touchscreen_t::yPos, index);
    icon.width  = _database.read(Database::Section::touchscreen_t::width, index);
    icon.height = _database.read(Database::Section::touchscreen_t::height, index);

    modelInstance().setIconState(icon, state);
}

void Touchscreen::processButton(const size_t buttonID, const bool state)
{
    bool   changeScreen = false;
    size_t newScreen    = 0;

    if (_database.read(Database::Section::touchscreen_t::pageSwitchEnabled, buttonID))
    {
        changeScreen = true;
        newScreen    = _database.read(Database::Section::touchscreen_t::pageSwitchIndex, buttonID);
    }

    if (_eventNotifier != nullptr)
        _eventNotifier->button(buttonID, state);

    // if the button should change screen, change it immediately
    // this will result in button never sending off state so do it manually first
    if (changeScreen)
    {
        if (_eventNotifier != nullptr)
            _eventNotifier->button(buttonID, false);

        setScreen(newScreen);
    }
}

bool Touchscreen::setBrightness(brightness_t brightness)
{
    if (!isInitialized(mode_t::normal))
        return false;

    return modelInstance().setBrightness(brightness);
}

void Touchscreen::processCoordinate(pressType_t pressType, uint16_t xPos, uint16_t yPos)
{
    for (size_t i = 0; i < MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS; i++)
    {
        if (_database.read(Database::Section::touchscreen_t::analogPage, i) == static_cast<int32_t>(activeScreen()))
        {
            uint16_t startXCoordinate = _database.read(Database::Section::touchscreen_t::analogStartXCoordinate, i);
            uint16_t endXCoordinate   = _database.read(Database::Section::touchscreen_t::analogEndXCoordinate, i);
            uint16_t startYCoordinate = _database.read(Database::Section::touchscreen_t::analogStartYCoordinate, i);
            uint16_t endYCoordinate   = _database.read(Database::Section::touchscreen_t::analogEndYCoordinate, i);

            uint16_t startCoordinate;
            uint16_t endCoordinate;
            uint16_t value;

            // x
            if (_database.read(Database::Section::touchscreen_t::analogType, i) == static_cast<int32_t>(IO::Touchscreen::analogType_t::horizontal))
            {
                value = xPos;

                if (pressType == pressType_t::hold)
                {
                    // y coordinate can be ignored once the touchscreen is pressed, verify and constrain x range only
                    if (value > endXCoordinate)
                        value = endXCoordinate;
                    else if (value < startXCoordinate)
                        value = startXCoordinate;
                }
                else if (pressType == pressType_t::initial)
                {
                    if (((value < startXCoordinate) || (value > endXCoordinate)) || ((yPos < startYCoordinate) || (yPos > endYCoordinate)))
                        continue;
                    else
                        _analogActive[i] = true;
                }
                else
                {
                    _analogActive[i] = false;
                }

                startCoordinate = startXCoordinate;
                endCoordinate   = endXCoordinate;
            }
            // y
            else
            {
                value = yPos;

                if (pressType == pressType_t::hold)
                {
                    // x coordinate can be ignored once the touchscreen is pressed, verify and constrain x range only
                    if (value > endYCoordinate)
                        value = endYCoordinate;
                    else if (value < startYCoordinate)
                        value = startYCoordinate;
                }
                else if (pressType == pressType_t::initial)
                {
                    if (((xPos < startXCoordinate) || (xPos > endXCoordinate)) || ((value < startYCoordinate) || (value > endYCoordinate)))
                        continue;
                    else
                        _analogActive[i] = true;
                }
                else
                {
                    _analogActive[i] = false;
                }

                startCoordinate = startYCoordinate;
                endCoordinate   = endYCoordinate;
            }

            // scale the value to ADC range
            if (_analogActive[i])
            {
                if (_eventNotifier != nullptr)
                    _eventNotifier->analog(i, value, startCoordinate, endCoordinate);
            }
            else
            {
                if (_database.read(Database::Section::touchscreen_t::analogResetOnRelease, i))
                {
                    if (_eventNotifier != nullptr)
                        _eventNotifier->analog(i, 0, startCoordinate, endCoordinate);
                }
            }
        }
    }
}

bool Touchscreen::isInitialized() const
{
    return _initialized;
}

bool Touchscreen::isInitialized(mode_t mode) const
{
    switch (mode)
    {
    case mode_t::normal:
        return _initialized && (_mode == mode);

    case mode_t::cdcPassthrough:
        return _mode == mode;

    default:
        return false;
    }
}

IO::TouchscreenBase& Touchscreen::modelInstance()
{
    switch (_activeModel)
    {
    case TouchscreenBase::model_t::viewtech:
        return _viewtech;

    default:
        return _nextion;
    }
}