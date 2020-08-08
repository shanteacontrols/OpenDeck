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

#pragma once

#include <inttypes.h>
#include <stdlib.h>
#include "database/Database.h"

namespace IO
{
    ///
    /// \brief Touchscreen control.
    /// \defgroup interfaceLCDTouch Touchscreen
    /// \ingroup interfaceLCD
    /// @{

    class Touchscreen
    {
        public:
        using icon_t = uint8_t;

        class Model
        {
            public:
            virtual bool init()                                              = 0;
            virtual bool setScreen(size_t screenID)                          = 0;
            virtual bool update(size_t& buttonID, bool& state)               = 0;
            virtual void setIconState(Touchscreen::icon_t& icon, bool state) = 0;
        };

        Touchscreen(Database& database, Model& model)
        {}

        enum class setting_t : uint8_t
        {
            enable,
            brightness,
            AMOUNT
        };

        bool init()
        {
            return false;
        }

        void update()
        {
        }

        void setScreen(size_t screenID)
        {
        }

        size_t activeScreen()
        {
            return 0;
        }

        void setButtonHandler(void (*fptr)(size_t index, bool state))
        {
        }

        void setScreenChangeHandler(void (*fptr)(size_t screenID))
        {
        }

        void setIconState(size_t index, bool state)
        {
        }
    };

    /// @}
}    // namespace IO