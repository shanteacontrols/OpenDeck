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
#include "core/src/general/RingBuffer.h"

#ifndef TOUCHSCREEN_SUPPORTED
#include "Stub.h"
#else

namespace IO
{
    class Touchscreen
    {
        public:
        typedef struct
        {
            uint16_t xPos;
            uint16_t yPos;
            uint16_t width;
            uint16_t height;
            uint16_t onScreen;
            uint16_t offScreen;
        } icon_t;

        typedef struct
        {
            uint16_t indexTS;
            uint16_t screen;
        } screenButton_t;

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
                static const size_t                          bufferSize = 100;
                static core::RingBuffer<uint8_t, bufferSize> rxBuffer;
            };

            enum class model_t : uint8_t
            {
                nextion,
                viewtech,
                AMOUNT
            };

            virtual bool init()                                              = 0;
            virtual bool deInit()                                            = 0;
            virtual bool setScreen(size_t screenID)                          = 0;
            virtual bool update(size_t& buttonID, bool& state)               = 0;
            virtual void setIconState(Touchscreen::icon_t& icon, bool state) = 0;
        };

        Touchscreen(Database& database)
            : database(database)
        {}

        enum class setting_t : uint8_t
        {
            enable,
            model,
            brightness,
            AMOUNT
        };

        bool   init();
        bool   deInit();
        bool   registerModel(IO::Touchscreen::Model::model_t, Model* ptr);
        void   update();
        void   setScreen(size_t screenID);
        size_t activeScreen();
        void   setButtonHandler(void (*fptr)(size_t index, bool state));
        void   setScreenChangeHandler(void (*fptr)(size_t screenID));
        void   setIconState(size_t index, bool state);

        private:
        bool isModelValid(Model::model_t model);

        Database& database;

        void (*buttonHandler)(size_t index, bool state)                = nullptr;
        void (*screenHandler)(size_t screenID)                         = nullptr;
        size_t  activeScreenID                                         = 0;
        bool    initialized                                            = false;
        Model*  modelPtr[static_cast<uint8_t>(Model::model_t::AMOUNT)] = {};
        uint8_t activeModel                                            = static_cast<uint8_t>(Model::model_t::AMOUNT);
    };
}    // namespace IO

#endif