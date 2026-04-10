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

#pragma once

#include "deps.h"
#include "drivers/driver_base.h"

namespace io::leds
{
    class HwaHw : public Hwa
    {
        public:
        explicit HwaHw(DriverBase& driver)
            : _driver(driver)
        {}

        void update() override
        {
            _driver.update();
        }

        void setState(size_t index, brightness_t brightness) override
        {
            _driver.setState(index, brightness);
        }

        size_t rgbFromOutput(size_t index) override
        {
            return _driver.rgbFromOutput(index);
        }

        size_t rgbComponentFromRgb(size_t index, rgbComponent_t component) override
        {
            return _driver.rgbComponentFromRgb(index, component);
        }

        private:
        DriverBase& _driver;
    };
}    // namespace io::leds
