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

#ifdef PROJECT_TARGET_SUPPORT_I2C

#include "i2c.h"

using namespace io::i2c;

std::array<Peripheral*, I2c::MAX_PERIPHERALS> I2c::_peripherals;
size_t                                        I2c::_peripheralCounter;

I2c::~I2c()
{
    for (size_t i = 0; i < I2c::MAX_PERIPHERALS; i++)
    {
        _peripherals.at(i) = nullptr;
    }

    _peripheralCounter = 0;
}

bool I2c::init()
{
    for (size_t i = 0; i < _peripheralCounter; i++)
    {
        if (_peripherals.at(i) != nullptr)
        {
            if (!_peripherals.at(i)->init())
            {
                return false;
            }
        }
    }

    return true;
}

void I2c::updateSingle(size_t index, bool forceRefresh)
{
    if (index >= maxComponentUpdateIndex())
    {
        return;
    }

    if (_peripherals.at(index) != nullptr)
    {
        _peripherals.at(index)->update();
    }
}

void I2c::updateAll(bool forceRefresh)
{
    for (size_t i = 0; i < _peripherals.size(); i++)
    {
        updateSingle(i, forceRefresh);
    }
}

size_t I2c::maxComponentUpdateIndex()
{
    return _peripherals.size();
}

void I2c::registerPeripheral(Peripheral* instance)
{
    if (_peripheralCounter >= MAX_PERIPHERALS)
    {
        return;
    }

    _peripherals[_peripheralCounter++] = instance;
}

#endif