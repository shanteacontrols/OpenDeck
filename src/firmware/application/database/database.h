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

#include "config.h"
#include "deps.h"
#include "application/system/config.h"

#include <type_traits>
#include <optional>

namespace database
{
    class Admin : public lib::lessdb::LessDb
    {
        public:
        Admin(Hwa&    hwa,
              Layout& layout);

        using sectionParameterType_t = lib::lessdb::sectionParameterType_t;
        using lib::lessdb::LessDb::read;
        using lib::lessdb::LessDb::update;

        template<typename T, typename I>
        uint32_t read(T section, I index)
        {
            auto blockIndex = BLOCK(section);
            auto value      = lib::lessdb::LessDb::read(static_cast<uint8_t>(blockIndex),
                                                   static_cast<uint8_t>(section),
                                                   static_cast<size_t>(index));

            return value;
        }

        template<typename I>
        uint32_t read(Config::Section::system_t section, I index)
        {
            return readSystemBlock(static_cast<size_t>(index));
        }

        template<typename T, typename I>
        bool read(T section, I index, uint32_t& value)
        {
            auto blockIndex = BLOCK(section);
            return lib::lessdb::LessDb::read(static_cast<uint8_t>(blockIndex),
                                             static_cast<uint8_t>(section),
                                             static_cast<size_t>(index),
                                             value);
        }

        template<typename I>
        bool read(Config::Section::system_t section, I index, uint32_t& value)
        {
            value = readSystemBlock(static_cast<size_t>(index));
            return true;
        }

        template<typename T, typename I, typename V>
        bool update(T section, I index, V value)
        {
            auto blockIndex = BLOCK(section);
            auto newValue   = static_cast<uint32_t>(value);

            return lib::lessdb::LessDb::update(static_cast<uint8_t>(blockIndex),
                                               static_cast<uint8_t>(section),
                                               static_cast<size_t>(index),
                                               newValue);
        }

        template<typename I, typename V>
        bool update(Config::Section::system_t section, I index, V value)
        {
            if (static_cast<uint8_t>(index) < static_cast<uint8_t>(Config::systemSetting_t::CUSTOM_SYSTEM_SETTING_START))
            {
                return false;
            }

            return updateSystemBlock(static_cast<size_t>(index), value);
        }

        bool    init();
        bool    init(Handlers& handlers);
        bool    factoryReset();
        uint8_t getSupportedPresets();
        bool    setPreset(uint8_t preset);
        uint8_t getPreset();
        bool    isInitialized();
        void    registerHandlers(Handlers& handlers);
        bool    setPresetPreserveState(bool state);
        bool    getPresetPreserveState();

        static constexpr Config::block_t BLOCK(Config::Section::global_t section)
        {
            return Config::block_t::GLOBAL;
        }

        static constexpr Config::block_t BLOCK(Config::Section::button_t section)
        {
            return Config::block_t::BUTTONS;
        }

        static constexpr Config::block_t BLOCK(Config::Section::encoder_t section)
        {
            return Config::block_t::ENCODERS;
        }

        static constexpr Config::block_t BLOCK(Config::Section::analog_t section)
        {
            return Config::block_t::ANALOG;
        }

        static constexpr Config::block_t BLOCK(Config::Section::leds_t section)
        {
            return Config::block_t::LEDS;
        }

        static constexpr Config::block_t BLOCK(Config::Section::i2c_t section)
        {
            return Config::block_t::I2C;
        }

        static constexpr Config::block_t BLOCK(Config::Section::touchscreen_t section)
        {
            return Config::block_t::TOUCHSCREEN;
        }

        private:
        Layout&   _layout;
        Handlers* _handlers = nullptr;

        const bool INITIALIZE_DATA;

        /// Address at which user data starts (after system block).
        /// Used to set correct offset in database for user layout.
        uint32_t _userDataStartAddress = 0;

        /// Address at which next preset should start.
        /// Used to calculate the start address of next preset.
        uint32_t _lastPresetAddress = 0;

        uint8_t  _activePreset     = 0;
        size_t   _supportedPresets = 0;
        uint16_t _uid              = 0;
        bool     _initialized      = false;

        void                   customInitGlobal();
        void                   customInitButtons();
        void                   customInitEncoders();
        void                   customInitAnalog();
        void                   customInitLEDs();
        void                   customInitDisplay();
        void                   customInitTouchscreen();
        bool                   isSignatureValid();
        bool                   setUID();
        bool                   setPresetInternal(uint8_t preset);
        uint16_t               readSystemBlock(size_t index);
        bool                   updateSystemBlock(size_t index, uint16_t value);
        std::optional<uint8_t> sysConfigGet(sys::Config::Section::global_t section, size_t index, uint16_t& value);
        std::optional<uint8_t> sysConfigSet(sys::Config::Section::global_t section, size_t index, uint16_t value);
    };

    template<typename... sections>
    class User
    {
        public:
        User(Admin& admin)
            : _admin(admin)
        {}

        using Config = database::Config;

        template<typename T, typename I>
        typename std::enable_if<(std::is_same_v<T, sections> || ...), uint32_t>::type
        read(T section, I index)
        {
            auto blockIndex = Admin::BLOCK(section);
            auto value      = _admin.read(static_cast<uint8_t>(blockIndex),
                                     static_cast<uint8_t>(section),
                                     static_cast<size_t>(index));

            return value;
        }

        template<typename T, typename I>
        typename std::enable_if<(std::is_same_v<T, sections> || ...), bool>::type
        read(T section, I index, uint32_t& value)
        {
            auto blockIndex = Admin::BLOCK(section);
            return _admin.read(static_cast<uint8_t>(blockIndex),
                               static_cast<uint8_t>(section),
                               static_cast<size_t>(index),
                               value);
        }

        template<typename T, typename I, typename V>
        typename std::enable_if<(std::is_same_v<T, sections> || ...), bool>::type
        update(T section, I index, V value)
        {
            auto blockIndex = Admin::BLOCK(section);
            auto newValue   = static_cast<uint32_t>(value);

            return _admin.update(static_cast<uint8_t>(blockIndex),
                                 static_cast<uint8_t>(section),
                                 static_cast<size_t>(index),
                                 newValue);
        }

        uint8_t getPreset()
        {
            return _admin.getPreset();
        }

        private:
        Admin& _admin;
    };
}    // namespace database
