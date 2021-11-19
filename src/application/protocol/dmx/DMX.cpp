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

#include "DMX.h"
#include "dmxusb/src/DMXUSBWidget.h"

#ifdef DMX_SUPPORTED

#include "io/common/Common.h"
#include "util/conversion/Conversion.h"
#include "util/configurable/Configurable.h"

using namespace IO;

Protocol::DMX::DMX(HWA& hwa, Database& database)
    : DMXUSBWidget(hwa)
    , _hwa(hwa)
    , _database(database)
{
    ConfigHandler.registerConfig(
        System::Config::block_t::global,
        // read
        [this](uint8_t section, size_t index, uint16_t& value) {
            return sysConfigGet(static_cast<System::Config::Section::global_t>(section), index, value);
        },

        // write
        [this](uint8_t section, size_t index, uint16_t value) {
            return sysConfigSet(static_cast<System::Config::Section::global_t>(section), index, value);
        });
}

bool Protocol::DMX::init()
{
    uniqueID_t uniqueID;
    _hwa.uniqueID(uniqueID);

    uint32_t dmxSerialNr = uniqueID[0];
    dmxSerialNr <<= 8;
    dmxSerialNr |= uniqueID[1];
    dmxSerialNr <<= 8;
    dmxSerialNr |= uniqueID[2];
    dmxSerialNr <<= 8;
    dmxSerialNr |= uniqueID[3];

    setWidgetInfo({ dmxSerialNr,
                    ESTA_ID,
                    0x00,
                    { SW_VERSION_MAJOR,
                      SW_VERSION_MINOR },
                    "Shantea Controls",
                    BOARD_STRING });

    if (_database.read(Database::Section::global_t::dmx, setting_t::enable))
        return ::DMXUSBWidget::init();

    return true;
}

void Protocol::DMX::read()
{
    if (_database.read(Database::Section::global_t::dmx, setting_t::enable))
        ::DMXUSBWidget::read();
}

std::optional<uint8_t> Protocol::DMX::sysConfigGet(System::Config::Section::global_t section, size_t index, uint16_t& value)
{
    int32_t readValue = 0;
    uint8_t result    = System::Config::status_t::errorRead;

    switch (section)
    {
    case System::Config::Section::global_t::dmx:
    {
        bool dmxEnabled = _database.read(Util::Conversion::sys2DBsection(section), setting_t::enable);

        if (!dmxEnabled && _hwa.allocated(IO::Common::interface_t::uart))
        {
            return System::Config::status_t::serialPeripheralAllocatedError;
        }
        else
        {
            if (_hwa.allocated(IO::Common::interface_t::cdc))
            {
                return System::Config::status_t::cdcAllocatedError;
            }

            result = _database.read(Util::Conversion::sys2DBsection(section), index, readValue) ? System::Config::status_t::ack : System::Config::status_t::errorRead;
        }
    }
    break;

    default:
        return std::nullopt;
    }

    value = readValue;
    return result;
}

std::optional<uint8_t> Protocol::DMX::sysConfigSet(System::Config::Section::global_t section, size_t index, uint16_t value)
{
    uint8_t result        = System::Config::status_t::errorWrite;
    auto    dmxInitAction = Common::initAction_t::asIs;

    switch (section)
    {
    case System::Config::Section::global_t::dmx:
    {
        bool dmxEnabled = _database.read(Util::Conversion::sys2DBsection(section), setting_t::enable);

        if (!dmxEnabled && _hwa.allocated(IO::Common::interface_t::uart))
        {
            result = System::Config::status_t::serialPeripheralAllocatedError;
        }
        else
        {
            if (_hwa.allocated(IO::Common::interface_t::cdc))
            {
                return System::Config::status_t::cdcAllocatedError;
            }

            dmxInitAction = value ? Common::initAction_t::init : Common::initAction_t::deInit;
            result        = System::Config::status_t::ack;
        }
    }
    break;

    default:
        return std::nullopt;
    }

    if (result == System::Config::status_t::ack)
    {
        result = _database.update(Util::Conversion::sys2DBsection(section), index, value) ? System::Config::status_t::ack : System::Config::status_t::errorWrite;

        switch (dmxInitAction)
        {
        case Common::initAction_t::init:
        {
            ::DMXUSBWidget::init();
        }
        break;

        case Common::initAction_t::deInit:
        {
            ::DMXUSBWidget::deInit();
        }
        break;

        default:
            break;
        }
    }

    return result;
}

#endif