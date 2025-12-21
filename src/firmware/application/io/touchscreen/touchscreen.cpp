/*

Copyright Igor Petrovic

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

#ifdef PROJECT_TARGET_SUPPORT_TOUCHSCREEN

#include "touchscreen.h"
#include "application/util/conversion/conversion.h"
#include "application/util/configurable/configurable.h"

#include "core/mcu.h"
#include "core/util/util.h"

using namespace io::touchscreen;

std::array<Model*, static_cast<size_t>(model_t::AMOUNT)> Touchscreen::_models;
uint8_t                                                  Model::_rxBuffer[Model::BUFFER_SIZE];
size_t                                                   Model::_bufferCount;

Touchscreen::Touchscreen(Hwa&      hwa,
                         Database& database)
    : _hwa(hwa)
    , _database(database)
{
    MidiDispatcher.listen(messaging::eventType_t::TOUCHSCREEN_LED,
                          [this](const messaging::Event& event)
                          {
                              setIconState(event.componentIndex, event.value);
                          });

    MidiDispatcher.listen(messaging::eventType_t::SYSTEM,
                          [this](const messaging::Event& event)
                          {
                              switch (event.systemMessage)
                              {
                              case messaging::systemMessage_t::PRESET_CHANGED:
                              {
                                  if (!init())
                                  {
                                      deInit();
                                  }
                              }
                              break;

                              default:
                                  break;
                              }
                          });

    ConfigHandler.registerConfig(
        sys::Config::block_t::TOUCHSCREEN,
        // read
        [this](uint8_t section, size_t index, uint16_t& value)
        {
            return sysConfigGet(static_cast<sys::Config::Section::touchscreen_t>(section), index, value);
        },

        // write
        [this](uint8_t section, size_t index, uint16_t value)
        {
            return sysConfigSet(static_cast<sys::Config::Section::touchscreen_t>(section), index, value);
        });
}

Touchscreen::~Touchscreen()
{
    for (size_t i = 0; i < static_cast<size_t>(model_t::AMOUNT); i++)
    {
        _models.at(i) = nullptr;
    }
}

bool Touchscreen::init()
{
    if (_database.read(database::Config::Section::touchscreen_t::SETTING, touchscreen::setting_t::ENABLE))
    {
        auto dbModel = static_cast<model_t>(_database.read(database::Config::Section::touchscreen_t::SETTING,
                                                           touchscreen::setting_t::MODEL));

        if (_initialized)
        {
            if (dbModel == _activeModel)
            {
                // nothing to do, same model already _initialized
                return true;
            }

            if (!deInit())
            {
                return false;
            }
        }

        _activeModel  = dbModel;
        auto instance = modelInstance(_activeModel);

        if (instance != nullptr)
        {
            _initialized = instance->init();
        }

        if (_initialized)
        {
            setScreen(_database.read(database::Config::Section::touchscreen_t::SETTING,
                                     touchscreen::setting_t::INITIAL_SCREEN));

            setBrightness(static_cast<brightness_t>(_database.read(database::Config::Section::touchscreen_t::SETTING,
                                                                   touchscreen::setting_t::BRIGHTNESS)));

            return true;
        }

        return false;
    }

    return false;
}

bool Touchscreen::deInit()
{
    if (!_initialized)
    {
        return true;    // nothing to do
    }

    auto ptr = modelInstance(_activeModel);

    if (ptr == nullptr)
    {
        return false;
    }

    if (ptr->deInit())
    {
        _initialized = false;
        return true;
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
    if (!isInitialized())
    {
        return;
    }

    auto ptr = modelInstance(_activeModel);

    if (ptr == nullptr)
    {
        return;
    }

    Data      data  = {};
    tsEvent_t event = ptr->update(data);

    switch (event)
    {
    case tsEvent_t::BUTTON:
    {
        processButton(data.buttonIndex, data.buttonState);
    }
    break;

    default:
        break;
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
/// param [in]: index  Index of screen to display.
void Touchscreen::setScreen(size_t index)
{
    if (!isInitialized())
    {
        return;
    }

    auto ptr = modelInstance(_activeModel);

    if (ptr == nullptr)
    {
        return;
    }

    ptr->setScreen(index);
    _activeScreenID = index;
    screenChangeHandler(index);
}

/// Used to retrieve currently active screen on display.
/// returns: Active display screen.
size_t Touchscreen::activeScreen()
{
    return _activeScreenID;
}

void Touchscreen::setIconState(size_t index, bool state)
{
    if (!isInitialized())
    {
        return;
    }

    if (index >= Collection::SIZE())
    {
        return;
    }

    auto ptr = modelInstance(_activeModel);

    if (ptr == nullptr)
    {
        return;
    }

    Icon icon      = {};
    icon.onScreen  = _database.read(database::Config::Section::touchscreen_t::ON_SCREEN, index);
    icon.offScreen = _database.read(database::Config::Section::touchscreen_t::OFF_SCREEN, index);

    if (icon.onScreen == icon.offScreen)
    {
        return;    // invalid screen indexes
    }

    if ((_activeScreenID != icon.onScreen) && (_activeScreenID != icon.offScreen))
    {
        return;    // don't allow setting icon on wrong screen
    }

    icon.xPos   = _database.read(database::Config::Section::touchscreen_t::X_POS, index);
    icon.yPos   = _database.read(database::Config::Section::touchscreen_t::Y_POS, index);
    icon.width  = _database.read(database::Config::Section::touchscreen_t::WIDTH, index);
    icon.height = _database.read(database::Config::Section::touchscreen_t::HEIGHT, index);

    ptr->setIconState(icon, state);
}

void Touchscreen::processButton(const size_t buttonIndex, const bool state)
{
    bool   changeScreen = false;
    size_t newScreen    = 0;

    if (_database.read(database::Config::Section::touchscreen_t::PAGE_SWITCH_ENABLED, buttonIndex))
    {
        changeScreen = true;
        newScreen    = _database.read(database::Config::Section::touchscreen_t::PAGE_SWITCH_INDEX, buttonIndex);
    }

    buttonHandler(buttonIndex, state);

    // if the button should change screen, change it immediately
    // this will result in button never sending off state so do it manually first
    if (changeScreen)
    {
        buttonHandler(buttonIndex, false);
        setScreen(newScreen);
    }
}

bool Touchscreen::setBrightness(brightness_t brightness)
{
    if (!isInitialized())
    {
        return false;
    }

    auto ptr = modelInstance(_activeModel);

    if (ptr == nullptr)
    {
        return false;
    }

    return ptr->setBrightness(brightness);
}

bool Touchscreen::isInitialized() const
{
    return _initialized;
}

Model* Touchscreen::modelInstance(model_t model)
{
    return _models[static_cast<size_t>(model)];
}

void Touchscreen::buttonHandler(size_t index, bool state)
{
    messaging::Event event = {};
    event.componentIndex   = index;
    event.value            = state;

    // mark this as forwarding message type - further action/processing is required
    MidiDispatcher.notify(messaging::eventType_t::TOUCHSCREEN_BUTTON, event);
}

void Touchscreen::screenChangeHandler(size_t index)
{
    messaging::Event event = {};
    event.componentIndex   = index;

    // mark this as forwarding message type - further action/processing is required
    MidiDispatcher.notify(messaging::eventType_t::TOUCHSCREEN_SCREEN, event);
}

std::optional<uint8_t> Touchscreen::sysConfigGet(sys::Config::Section::touchscreen_t section, size_t index, uint16_t& value)
{
    if (!isInitialized() && _hwa.allocated(io::common::Allocatable::interface_t::UART))
    {
        return sys::Config::Status::SERIAL_PERIPHERAL_ALLOCATED_ERROR;
    }

    uint32_t readValue;

    auto result = _database.read(util::Conversion::SYS_2_DB_SECTION(section), index, readValue)
                      ? sys::Config::Status::ACK
                      : sys::Config::Status::ERROR_READ;

    value = readValue;
    return result;
}

std::optional<uint8_t> Touchscreen::sysConfigSet(sys::Config::Section::touchscreen_t section, size_t index, uint16_t value)
{
    if (!isInitialized() && _hwa.allocated(io::common::Allocatable::interface_t::UART))
    {
        return sys::Config::Status::SERIAL_PERIPHERAL_ALLOCATED_ERROR;
    }

    auto initAction = common::initAction_t::AS_IS;
    bool writeToDb  = true;

    switch (section)
    {
    case sys::Config::Section::touchscreen_t::SETTING:
    {
        switch (index)
        {
        case static_cast<size_t>(setting_t::ENABLE):
        {
            if (value)
            {
                initAction = common::initAction_t::INIT;
            }
            else
            {
                initAction = common::initAction_t::DE_INIT;
            }
        }
        break;

        case static_cast<size_t>(setting_t::MODEL):
        {
            if (value >= static_cast<size_t>(model_t::AMOUNT))
            {
                return sys::Config::Status::ERROR_NEW_VALUE;
            }

            initAction = common::initAction_t::INIT;
        }
        break;

        case static_cast<size_t>(setting_t::BRIGHTNESS):
        {
            if (isInitialized())
            {
                if (!setBrightness(static_cast<brightness_t>(value)))
                {
                    return sys::Config::Status::ERROR_WRITE;
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
        result = _database.update(util::Conversion::SYS_2_DB_SECTION(section), index, value);
    }

    if (result)
    {
        if (initAction == common::initAction_t::INIT)
        {
            init();
        }
        else if (initAction == common::initAction_t::DE_INIT)
        {
            deInit();
        }

        return sys::Config::Status::ACK;
    }

    return sys::Config::Status::ERROR_WRITE;
}

#endif