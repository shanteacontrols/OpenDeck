/*

Copyright 2015-2022 Igor Petrovic

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

#include "dbms/src/LESSDB.h"
#include "system/Config.h"
#include "Config.h"

namespace Database
{
    class Instance : public LESSDB
    {
        public:
        class Handlers
        {
            public:
            virtual void presetChange(uint8_t preset) = 0;
            virtual void factoryResetStart()          = 0;
            virtual void factoryResetDone()           = 0;
            virtual void initialized()                = 0;
        };

        // Database has circular dependency problem: to define layout, details are needed
        // from almost all application modules. At the same time, nearly all modules need database.
        // To circumvent this, Layout class is defined which needs to be injected when
        // constructing database object. This way, layout can live somewhere else and
        // include anything it needs without affecting the main instance.
        class Layout
        {
            public:
            enum class type_t : uint8_t
            {
                system,
                user,
                full
            };

            virtual std::vector<Block>& layout(type_t type) = 0;
        };

        Instance(LESSDB::StorageAccess& storageAccess,
                 Layout&                layout,
                 bool                   initializeData);

        using sectionParameterType_t = LESSDB::sectionParameterType_t;
        using LESSDB::read;
        using LESSDB::update;

        template<typename T, typename I>
        int32_t read(T section, I index)
        {
            auto blockIndex = block(section);
            auto value      = LESSDB::read(static_cast<uint8_t>(blockIndex),
                                      static_cast<uint8_t>(section),
                                      static_cast<size_t>(index));

            // Workaround: to save space in database, MIDI channels are defined as half-byte
            // values, which means they have 0-15 range. MIDI channels, however, use 1-16 range.
            // When reading the channels from database, increase their value by 1.
            if constexpr (std::is_same<T, Config::Section::analog_t>::value)
            {
                if (section == Config::Section::analog_t::midiChannel)
                {
                    value++;
                }
            }

            if constexpr (std::is_same<T, Config::Section::button_t>::value)
            {
                if (section == Config::Section::button_t::midiChannel)
                {
                    value++;
                }
            }

            if constexpr (std::is_same<T, Config::Section::encoder_t>::value)
            {
                if (section == Config::Section::encoder_t::midiChannel)
                {
                    value++;
                }
            }

            if constexpr (std::is_same<T, Config::Section::leds_t>::value)
            {
                if (section == Config::Section::leds_t::midiChannel)
                {
                    value++;
                }
            }

            return value;
        }

        template<typename T, typename I>
        bool read(T section, I index, int32_t& value)
        {
            auto blockIndex = block(section);
            auto ret        = LESSDB::read(static_cast<uint8_t>(blockIndex),
                                    static_cast<uint8_t>(section),
                                    static_cast<size_t>(index),
                                    value);

            if constexpr (std::is_same<T, Config::Section::analog_t>::value)
            {
                if (section == Config::Section::analog_t::midiChannel)
                {
                    value++;
                }
            }

            if constexpr (std::is_same<T, Config::Section::button_t>::value)
            {
                if (section == Config::Section::button_t::midiChannel)
                {
                    value++;
                }
            }

            if constexpr (std::is_same<T, Config::Section::encoder_t>::value)
            {
                if (section == Config::Section::encoder_t::midiChannel)
                {
                    value++;
                }
            }

            if constexpr (std::is_same<T, Config::Section::leds_t>::value)
            {
                if (section == Config::Section::leds_t::midiChannel)
                {
                    value++;
                }
            }

            return ret;
        }

        template<typename T, typename I, typename V>
        bool update(T section, I index, V value)
        {
            auto blockIndex = block(section);
            auto newValue   = static_cast<int32_t>(value);

            // Workaround: to save space in database, MIDI channels are defined as half-byte
            // values, which means they have 0-15 range. MIDI channels, however, use 1-16 range.
            // When updating the channels in database, decrease their value by 1.
            if constexpr (std::is_same<T, Config::Section::analog_t>::value)
            {
                if (section == Config::Section::analog_t::midiChannel)
                {
                    newValue--;
                }
            }

            if constexpr (std::is_same<T, Config::Section::button_t>::value)
            {
                if (section == Config::Section::button_t::midiChannel)
                {
                    newValue--;
                }
            }

            if constexpr (std::is_same<T, Config::Section::encoder_t>::value)
            {
                if (section == Config::Section::encoder_t::midiChannel)
                {
                    newValue--;
                }
            }

            if constexpr (std::is_same<T, Config::Section::leds_t>::value)
            {
                if (section == Config::Section::leds_t::midiChannel)
                {
                    newValue--;
                }
            }

            return LESSDB::update(static_cast<uint8_t>(blockIndex),
                                  static_cast<uint8_t>(section),
                                  static_cast<size_t>(index),
                                  newValue);
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

        private:
        Config::block_t block(Config::Section::global_t section)
        {
            return Config::block_t::global;
        }

        Config::block_t block(Config::Section::button_t section)
        {
            return Config::block_t::buttons;
        }

        Config::block_t block(Config::Section::encoder_t section)
        {
            return Config::block_t::encoders;
        }

        Config::block_t block(Config::Section::analog_t section)
        {
            return Config::block_t::analog;
        }

        Config::block_t block(Config::Section::leds_t section)
        {
            return Config::block_t::leds;
        }

        Config::block_t block(Config::Section::i2c_t section)
        {
            return Config::block_t::i2c;
        }

        Config::block_t block(Config::Section::touchscreen_t section)
        {
            return Config::block_t::touchscreen;
        }

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
        std::optional<uint8_t> sysConfigGet(System::Config::Section::global_t section, size_t index, uint16_t& value);
        std::optional<uint8_t> sysConfigSet(System::Config::Section::global_t section, size_t index, uint16_t value);

        Handlers* _handlers = nullptr;
        Layout&   _layout;

        const bool _initializeData;

        /// Holds currently active preset.
        uint8_t _activePreset = 0;

        /// Address at which user data starts (after system block).
        /// Used to set correct offset in database for user layout.
        uint32_t _userDataStartAddress;

        /// Address at which next preset should start.
        /// Used to calculate the start address of next preset.
        uint32_t _lastPresetAddress;

        /// Holds total number of supported presets.
        uint16_t _supportedPresets;

        uint16_t _uid = 0;

        bool _initialized = false;
    };
}    // namespace Database
