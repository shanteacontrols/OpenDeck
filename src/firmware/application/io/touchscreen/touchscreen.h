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
#include "application/system/config.h"
#include "application/messaging/messaging.h"
#include "application/io/base.h"

#include <optional>

#ifdef PROJECT_TARGET_SUPPORT_TOUCHSCREEN

namespace io::touchscreen
{
    class Collection : public common::BaseCollection<PROJECT_TARGET_SUPPORTED_NR_OF_TOUCHSCREEN_COMPONENTS>
    {
        public:
        Collection() = delete;
    };

    class Touchscreen : public io::Base
    {
        public:
        Touchscreen(Hwa&      hwa,
                    Database& database);

        ~Touchscreen();

        bool        init() override;
        void        updateSingle(size_t index, bool forceRefresh = false) override;
        void        updateAll(bool forceRefresh = false) override;
        size_t      maxComponentUpdateIndex() override;
        static void registerModel(model_t model, Model* instance);

        private:
        Hwa&                                                            _hwa;
        Database&                                                       _database;
        size_t                                                          _activeScreenID = 0;
        bool                                                            _initialized    = false;
        model_t                                                         _activeModel    = model_t::AMOUNT;
        static std::array<Model*, static_cast<size_t>(model_t::AMOUNT)> _models;

        bool                   deInit();
        Model*                 modelInstance(model_t model);
        bool                   isInitialized() const;
        void                   setScreen(size_t index);
        size_t                 activeScreen();
        void                   setIconState(size_t index, bool state);
        bool                   setBrightness(brightness_t brightness);
        void                   processButton(const size_t buttonIndex, const bool state);
        void                   buttonHandler(size_t index, bool state);
        void                   screenChangeHandler(size_t index);
        std::optional<uint8_t> sysConfigGet(sys::Config::Section::touchscreen_t section, size_t index, uint16_t& value);
        std::optional<uint8_t> sysConfigSet(sys::Config::Section::touchscreen_t section, size_t index, uint16_t value);
    };
}    // namespace io::touchscreen

#else
#include "stub.h"
#endif