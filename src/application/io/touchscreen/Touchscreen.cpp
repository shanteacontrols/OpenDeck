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

#ifdef HW_SUPPORT_TOUCHSCREEN

#include "Touchscreen.h"
#include "core/src/util/Util.h"
#include "core/src/Timing.h"
#include "util/conversion/Conversion.h"
#include "util/configurable/Configurable.h"

using namespace IO;

std::array<Touchscreen::Model*, static_cast<size_t>(Touchscreen::model_t::AMOUNT)> Touchscreen::_models;

Touchscreen::Touchscreen(HWA&            hwa,
                         Database&       database,
                         CDCPassthrough& cdcPassthrough)
    : _hwa(hwa)
    , _database(database)
    , _cdcPassthrough(cdcPassthrough)
{
    MIDIDispatcher.listen(Messaging::eventType_t::TOUCHSCREEN_LED,
                          [this](const Messaging::event_t& event)
                          {
                              setIconState(event.componentIndex, event.value);
                          });

    MIDIDispatcher.listen(Messaging::eventType_t::SYSTEM,
                          [this](const Messaging::event_t& event)
                          {
                              switch (event.systemMessage)
                              {
                              case Messaging::systemMessage_t::PRESET_CHANGED:
                              {
                                  if (!init(mode_t::NORMAL))
                                  {
                                      deInit(mode_t::NORMAL);
                                  }
                              }
                              break;

                              default:
                                  break;
                              }
                          });

    ConfigHandler.registerConfig(
        System::Config::block_t::TOUCHSCREEN,
        // read
        [this](uint8_t section, size_t index, uint16_t& value)
        {
            return sysConfigGet(static_cast<System::Config::Section::touchscreen_t>(section), index, value);
        },

        // write
        [this](uint8_t section, size_t index, uint16_t value)
        {
            return sysConfigSet(static_cast<System::Config::Section::touchscreen_t>(section), index, value);
        });
}

Touchscreen::~Touchscreen()
{
    for (size_t i = 0; i < static_cast<size_t>(Touchscreen::model_t::AMOUNT); i++)
    {
        _models.at(i) = nullptr;
    }
}

bool Touchscreen::init()
{
    return init(mode_t::NORMAL);
}

bool Touchscreen::init(mode_t mode)
{
    switch (mode)
    {
    case mode_t::NORMAL:
    {
        if (_database.read(Database::Config::Section::touchscreen_t::SETTING, Touchscreen::setting_t::ENABLE))
        {
            auto dbModel = static_cast<model_t>(_database.read(Database::Config::Section::touchscreen_t::SETTING,
                                                               Touchscreen::setting_t::MODEL));

            if (_initialized)
            {
                if (dbModel == _activeModel)
                {
                    // nothing to do, same model already _initialized
                    return true;
                }

                if (!deInit(mode))
                {
                    return false;
                }
            }

            if (_cdcPassthrough.deInit())
            {
                _activeModel  = dbModel;
                auto instance = modelInstance(_activeModel);

                if (instance != nullptr)
                {
                    _initialized = instance->init();
                }

                if (_initialized)
                {
                    setScreen(_database.read(Database::Config::Section::touchscreen_t::SETTING,
                                             Touchscreen::setting_t::INITIAL_SCREEN));

                    setBrightness(static_cast<brightness_t>(_database.read(Database::Config::Section::touchscreen_t::SETTING,
                                                                           Touchscreen::setting_t::BRIGHTNESS)));

                    _mode = mode;

                    return true;
                }

                return false;
            }

            return false;
        }
    }
    break;

    case mode_t::CDC_PASSTHROUGH:
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
    case mode_t::NORMAL:
    {
        if (!_initialized)
        {
            return true;    // nothing to do
        }

        if (_cdcPassthrough.deInit())
        {
            if (modelInstance(_activeModel)->deInit())
            {
                _mode        = mode_t::NORMAL;
                _initialized = false;

                return true;
            }
        }
    }
    break;

    case mode_t::CDC_PASSTHROUGH:
    {
        if (_cdcPassthrough.deInit())
        {
            _mode = mode_t::NORMAL;
            return true;
        }
    }
    break;

    default:
        return false;
    }

    return false;
}

void Touchscreen::updateSingle(size_t index, bool forceRefresh)
{
    // ignore index here - not applicable
    updateAll(forceRefresh);
}

void Touchscreen::updateAll(bool forceRefresh)
{
    if (isInitialized(mode_t::NORMAL))
    {
        tsData_t  tsData;
        tsEvent_t event = modelInstance(_activeModel)->update(tsData);

        switch (event)
        {
        case tsEvent_t::BUTTON:
        {
            processButton(tsData.buttonID, tsData.buttonState);
        }
        break;

        default:
            break;
        }
    }
    else if (isInitialized(mode_t::CDC_PASSTHROUGH))
    {
        auto touchscreenToUSB = [&]()
        {
            uint32_t size = 0;

            for (size = 0; size < BUFFER_SIZE_TSCREEN_CDC_PASSTHROUGH; size++)
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

        auto usbToTouchscreen = [&]()
        {
            size_t size = 0;

            while (_cdcPassthrough.cdcRead(_rxBuffer, size, BUFFER_SIZE_TSCREEN_CDC_PASSTHROUGH))
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

size_t Touchscreen::maxComponentUpdateIndex()
{
    return 0;
}

void Touchscreen::registerModel(model_t model, Model* instance)
{
    _models[static_cast<size_t>(model)] = instance;
}

/// Switches to requested screen on display
/// param [in]: screenID  Index of screen to display.
void Touchscreen::setScreen(size_t screenID)
{
    if (!isInitialized(mode_t::NORMAL))
    {
        return;
    }

    modelInstance(_activeModel)->setScreen(screenID);
    _activeScreenID = screenID;
    screenChangeHandler(screenID);
}

/// Used to retrieve currently active screen on display.
/// returns: Active display screen.
size_t Touchscreen::activeScreen()
{
    return _activeScreenID;
}

void Touchscreen::setIconState(size_t index, bool state)
{
    if (!isInitialized(mode_t::NORMAL))
    {
        return;
    }

    if (index >= Collection::SIZE())
    {
        return;
    }

    icon_t icon;

    icon.onScreen  = _database.read(Database::Config::Section::touchscreen_t::ON_SCREEN, index);
    icon.offScreen = _database.read(Database::Config::Section::touchscreen_t::OFF_SCREEN, index);

    if (icon.onScreen == icon.offScreen)
    {
        return;    // invalid screen indexes
    }

    if ((_activeScreenID != icon.onScreen) && (_activeScreenID != icon.offScreen))
    {
        return;    // don't allow setting icon on wrong screen
    }

    icon.xPos   = _database.read(Database::Config::Section::touchscreen_t::X_POS, index);
    icon.yPos   = _database.read(Database::Config::Section::touchscreen_t::Y_POS, index);
    icon.width  = _database.read(Database::Config::Section::touchscreen_t::WIDTH, index);
    icon.height = _database.read(Database::Config::Section::touchscreen_t::HEIGHT, index);

    modelInstance(_activeModel)->setIconState(icon, state);
}

void Touchscreen::processButton(const size_t buttonID, const bool state)
{
    bool   changeScreen = false;
    size_t newScreen    = 0;

    if (_database.read(Database::Config::Section::touchscreen_t::PAGE_SWITCH_ENABLED, buttonID))
    {
        changeScreen = true;
        newScreen    = _database.read(Database::Config::Section::touchscreen_t::PAGE_SWITCH_INDEX, buttonID);
    }

    buttonHandler(buttonID, state);

    // if the button should change screen, change it immediately
    // this will result in button never sending off state so do it manually first
    if (changeScreen)
    {
        buttonHandler(buttonID, false);
        setScreen(newScreen);
    }
}

bool Touchscreen::setBrightness(brightness_t brightness)
{
    if (!isInitialized(mode_t::NORMAL))
    {
        return false;
    }

    return modelInstance(_activeModel)->setBrightness(brightness);
}

bool Touchscreen::isInitialized() const
{
    return _initialized;
}

bool Touchscreen::isInitialized(mode_t mode) const
{
    switch (mode)
    {
    case mode_t::NORMAL:
        return _initialized && (_mode == mode);

    case mode_t::CDC_PASSTHROUGH:
        return _mode == mode;

    default:
        return false;
    }
}

Touchscreen::Model* Touchscreen::modelInstance(model_t model)
{
    return _models[static_cast<size_t>(model)];
}

void Touchscreen::buttonHandler(size_t index, bool state)
{
    Messaging::event_t event;

    event.componentIndex = index;
    event.value          = state;

    // mark this as forwarding message type - further action/processing is required
    MIDIDispatcher.notify(Messaging::eventType_t::TOUCHSCREEN_BUTTON, event);
}

void Touchscreen::screenChangeHandler(size_t screenID)
{
    Messaging::event_t event;

    event.componentIndex = screenID;

    // mark this as forwarding message type - further action/processing is required
    MIDIDispatcher.notify(Messaging::eventType_t::TOUCHSCREEN_SCREEN, event);
}

std::optional<uint8_t> Touchscreen::sysConfigGet(System::Config::Section::touchscreen_t section, size_t index, uint16_t& value)
{
    if (!isInitialized() && _hwa.allocated(IO::Common::Allocatable::interface_t::UART))
    {
        return System::Config::status_t::SERIAL_PERIPHERAL_ALLOCATED_ERROR;
    }

    switch (section)
    {
    case System::Config::Section::touchscreen_t::SETTING:
    {
        switch (index)
        {
        case static_cast<size_t>(setting_t::CDC_PASSTHROUGH):
        {
            if (_cdcPassthrough.allocated(IO::Common::Allocatable::interface_t::CDC))
            {
                return System::Config::status_t::CDC_ALLOCATED_ERROR;
            }
        }
        break;

        default:
            break;
        }
    }
    break;

    default:
        break;
    }

    uint32_t readValue;

    auto result = _database.read(Util::Conversion::SYS_2_DB_SECTION(section), index, readValue)
                      ? System::Config::status_t::ACK
                      : System::Config::status_t::ERROR_READ;

    value = readValue;
    return result;
}

std::optional<uint8_t> Touchscreen::sysConfigSet(System::Config::Section::touchscreen_t section, size_t index, uint16_t value)
{
    if (!isInitialized() && _hwa.allocated(IO::Common::Allocatable::interface_t::UART))
    {
        return System::Config::status_t::SERIAL_PERIPHERAL_ALLOCATED_ERROR;
    }

    auto initAction = Common::initAction_t::AS_IS;
    auto mode       = mode_t::NORMAL;
    bool writeToDb  = true;

    switch (section)
    {
    case System::Config::Section::touchscreen_t::SETTING:
    {
        switch (index)
        {
        case static_cast<size_t>(setting_t::ENABLE):
        {
            if (value)
            {
                initAction = Common::initAction_t::INIT;
            }
            else
            {
                initAction = Common::initAction_t::DE_INIT;
            }
        }
        break;

        case static_cast<size_t>(setting_t::MODEL):
        {
            if (value >= static_cast<size_t>(model_t::AMOUNT))
            {
                return System::Config::status_t::ERROR_NEW_VALUE;
            }

            initAction = Common::initAction_t::INIT;
        }
        break;

        case static_cast<size_t>(setting_t::BRIGHTNESS):
        {
            if (isInitialized())
            {
                if (!setBrightness(static_cast<brightness_t>(value)))
                {
                    return System::Config::status_t::ERROR_WRITE;
                }
            }
        }
        break;

        case static_cast<size_t>(setting_t::INITIAL_SCREEN):
        {
            if (isInitialized())
            {
                setScreen(value);
            }
        }
        break;

        case static_cast<size_t>(IO::Touchscreen::setting_t::CDC_PASSTHROUGH):
        {
            if (_cdcPassthrough.allocated(IO::Common::Allocatable::interface_t::CDC))
            {
                return System::Config::status_t::CDC_ALLOCATED_ERROR;
            }

            mode       = mode_t::CDC_PASSTHROUGH;
            initAction = value ? Common::initAction_t::INIT : Common::initAction_t::DE_INIT;
            writeToDb  = false;
        }
        break;

        default:
            break;
        }
    }
    break;

    default:
        break;
    }

    bool result = true;

    if (writeToDb)
    {
        result = _database.update(Util::Conversion::SYS_2_DB_SECTION(section), index, value);
    }

    if (result)
    {
        if (initAction == Common::initAction_t::INIT)
        {
            init(mode);
        }
        else if (initAction == Common::initAction_t::DE_INIT)
        {
            deInit(mode);
        }

        return System::Config::status_t::ACK;
    }

    return System::Config::status_t::ERROR_WRITE;
}

#endif