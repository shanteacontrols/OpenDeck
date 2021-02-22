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
#include "io/common/CInfo.h"

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
            button,
            coordinate
        };

        enum class pressType_t : uint8_t
        {
            none,
            initial,
            hold
        };

        enum class setting_t : uint8_t
        {
            enable,
            model,
            brightness,
            initialScreen,
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

        enum class analogType_t : uint8_t
        {
            horizontal,
            vertical
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
            uint16_t    xPos        = 0;
            uint16_t    yPos        = 0;
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

        class EventNotifier
        {
            public:
            virtual void button(size_t index, bool state)                                 = 0;
            virtual void analog(size_t index, uint16_t value, uint16_t min, uint16_t max) = 0;
            virtual void screenChange(size_t screenID)                                    = 0;
        };

        Touchscreen(Database&      database,
                    ComponentInfo& cInfo)
            : _database(database)
            , _cInfo(cInfo)
        {}

        bool   init();
        bool   deInit();
        bool   registerModel(IO::Touchscreen::Model::model_t, Model* ptr);
        void   update();
        void   setScreen(size_t screenID);
        size_t activeScreen();
        void   registerEventNotifier(EventNotifier& eventNotifer);
        void   setIconState(size_t index, bool state);
        bool   setBrightness(brightness_t brightness);

        private:
        bool isModelValid(Model::model_t model);
        void processButton(const size_t buttonID, const bool state);
        void processCoordinate(pressType_t pressType, uint16_t xPos, uint16_t yPos);

        Database&      _database;
        ComponentInfo& _cInfo;
        EventNotifier* _eventNotifier = nullptr;

        size_t  _activeScreenID                                         = 0;
        bool    _initialized                                            = false;
        Model*  _modelPtr[static_cast<uint8_t>(Model::model_t::AMOUNT)] = {};
        uint8_t _activeModel                                            = static_cast<uint8_t>(Model::model_t::AMOUNT);
        bool    _analogActive[MAX_NUMBER_OF_TOUCHSCREEN_COMPONENTS]     = {};
    };
}    // namespace IO

#endif