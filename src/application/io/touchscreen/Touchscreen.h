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

#pragma once

#include <inttypes.h>
#include <stdlib.h>
#include "database/Database.h"
#include "io/buttons/Buttons.h"

#ifndef TOUCHSCREEN_SUPPORTED
#include "Stub.h"
#else

namespace IO
{
    class Touchscreen
    {
        public:
        enum class tsEvent_t : uint8_t
        {
            none,
            button
        };

        enum class pressType_t : uint8_t
        {
            none,
            initial,
            hold
        };

        enum class componentType_t : uint8_t
        {
            button,
            indicator,
            AMOUNT
        };

        enum class setting_t : uint8_t
        {
            enable,
            model,
            brightness,
            AMOUNT
        };

        enum class brightness_t : uint8_t
        {
            _10,
            _25,
            _50,
            _75,
            _80,
            _90,
            _100
        };

        struct icon_t
        {
            uint16_t xPos      = 0;
            uint16_t yPos      = 0;
            uint16_t width     = 0;
            uint16_t height    = 0;
            uint16_t onScreen  = 0;
            uint16_t offScreen = 0;
        };

        struct tsData_t
        {
            pressType_t pressType   = pressType_t::none;
            size_t      buttonID    = 0;
            bool        buttonState = false;
        };

        class Model
        {
            public:
            class HWA
            {
                public:
                virtual bool init()              = 0;
                virtual bool deInit()            = 0;
                virtual bool write(uint8_t data) = 0;
                virtual bool read(uint8_t& data) = 0;
            };

            class Common
            {
                public:
                Common() {}

                protected:
                static constexpr size_t bufferSize = 50;
                static uint8_t          rxBuffer[bufferSize];
                static size_t           bufferCount;
            };

            enum class model_t : uint8_t
            {
                nextion,
                viewtech,
                AMOUNT
            };

            virtual bool      init()                                              = 0;
            virtual bool      deInit()                                            = 0;
            virtual bool      setScreen(size_t screenID)                          = 0;
            virtual tsEvent_t update(tsData_t& tsData)                            = 0;
            virtual void      setIconState(Touchscreen::icon_t& icon, bool state) = 0;
            virtual bool      setBrightness(brightness_t brightness)              = 0;
        };

        Touchscreen(Database&      database,
                    IO::Buttons&   buttons,
                    ComponentInfo& cInfo)
            : database(database)
            , buttons(buttons)
            , cInfo(cInfo)
        {}

        bool   init();
        bool   deInit();
        bool   registerModel(IO::Touchscreen::Model::model_t, Model* ptr);
        void   update();
        void   setScreen(size_t screenID);
        size_t activeScreen();
        void   setScreenChangeHandler(void (*fptr)(size_t screenID));
        void   setIconState(size_t index, bool state);
        bool   setBrightness(brightness_t brightness);

        private:
        bool isModelValid(Model::model_t model);
        void processButton(const size_t buttonID, const bool state);

        Database&      database;
        IO::Buttons&   buttons;
        ComponentInfo& cInfo;

        void (*screenHandler)(size_t screenID)                         = nullptr;
        size_t  activeScreenID                                         = 0;
        bool    initialized                                            = false;
        Model*  modelPtr[static_cast<uint8_t>(Model::model_t::AMOUNT)] = {};
        uint8_t activeModel                                            = static_cast<uint8_t>(Model::model_t::AMOUNT);
    };
}    // namespace IO

#endif