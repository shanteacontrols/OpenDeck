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
        typedef struct
        {
            uint16_t xPos;
            uint16_t yPos;
            uint16_t width;
            uint16_t height;
            uint16_t onPage;
            uint16_t offPage;
        } icon_t;

        typedef struct
        {
            uint16_t indexTS;
            uint16_t page;
        } pageButton_t;

        class Model
        {
            public:
            class HWA
            {
                public:
                virtual bool init()              = 0;
                virtual bool write(uint8_t data) = 0;
                virtual bool read(uint8_t& data) = 0;
            };

            virtual bool init()                                                             = 0;
            virtual bool setScreen(uint8_t screenID)                                        = 0;
            virtual bool update(uint8_t& buttonID, bool& state)                             = 0;
            virtual void setIconState(Touchscreen::icon_t& icon, uint8_t index, bool state) = 0;
        };

        Touchscreen(Model& model)
            : model(model)
        {}

        bool    init();
        void    update();
        void    setScreen(uint8_t screenID);
        uint8_t activeScreen();
        void    setButtonHandler(void (*fptr)(uint8_t index, bool state));
        void    setIconState(uint8_t index, bool state);

        private:
        Model& model;

        void (*buttonHandler)(uint8_t index, bool state) = nullptr;

        static bool getIcon(size_t index, icon_t& icon);
        static bool isPageButton(size_t index, uint16_t& page);

        uint16_t activeScreenID = 0;
        bool     initialized    = false;
    };

    /// @}
}    // namespace IO