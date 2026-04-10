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

#include "application/io/base.h"
#include "application/io/digital/buttons/builder_test.h"
#include "application/io/digital/encoders/builder_test.h"
#include "application/database/builder_test.h"

namespace io::digital
{
    class DigitalTest : public io::Base
    {
        public:
        DigitalTest(io::buttons::Buttons&   buttons,
                    io::encoders::Encoders& encoders)
            : _buttons(buttons)
            , _encoders(encoders)
        {}

        bool init() override
        {
            return _buttons.init() && _encoders.init();
        }

        void updateSingle(size_t index, bool forceRefresh = false) override
        {
            (void)index;
            _buttons.updateAll(forceRefresh);
            _encoders.updateAll(forceRefresh);
        }

        void updateAll(bool forceRefresh = false) override
        {
            updateSingle(0, forceRefresh);
        }

        size_t maxComponentUpdateIndex() override
        {
            return 1;
        }

        private:
        io::buttons::Buttons&   _buttons;
        io::encoders::Encoders& _encoders;
    };

    class Builder
    {
        public:
        explicit Builder(database::Admin& database)
            : _builderButtons(database)
            , _builderEncoders(database)
            , _instance(_builderButtons._instance, _builderEncoders._instance)
        {}

        DigitalTest& instance()
        {
            return _instance;
        }

        io::buttons::Builder  _builderButtons;
        io::encoders::Builder _builderEncoders;
        DigitalTest           _instance;
    };
}    // namespace io::digital
