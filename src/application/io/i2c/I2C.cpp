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

#include "I2C.h"

using namespace IO;

std::array<I2C::Peripheral*, I2C::MAX_PERIPHERALS> I2C::_peripherals;
size_t                                             I2C::_peripheralCounter;

I2C::I2C(HWA& hwa)
    : _hwa(hwa)
{}

bool I2C::init()
{
    if (!_hwa.init())
        return false;

    // Get I2C addresses for each peripheral.
    // Once one of the provided addresses is found on I2C bus,
    // initialize the peripheral with that address.

    for (size_t i = 0; i < _peripheralCounter; i++)
    {
        if (_peripherals.at(i) != nullptr)
        {
            size_t totalAddresses  = 0;
            auto   addressesBuffer = _peripherals.at(i)->addresses(totalAddresses);

            for (size_t address = 0; address < totalAddresses; address++)
            {
                if (_hwa.deviceAvailable(addressesBuffer[address]))
                {
                    _peripherals.at(i)->init(addressesBuffer[address]);
                    break;
                }
            }
        }
    }

    return true;
}

void I2C::update(bool forceRefresh)
{
    for (size_t i = 0; i < _peripherals.size(); i++)
    {
        if (_peripherals.at(i) != nullptr)
            _peripherals.at(i)->update();
    }
}

void I2C::registerPeripheral(Peripheral* instance)
{
    if (_peripheralCounter >= MAX_PERIPHERALS)
        return;

    _peripherals[_peripheralCounter++] = instance;
}