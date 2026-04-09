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
    /**
     * @brief High-level database administrator for common and preset regions.
     *
     * This class owns the active LessDB instance, binds the common and preset
     * layouts to their storage regions, and exposes typed accessors used by
     * the rest of the application.
     */
    class Admin : public lib::lessdb::LessDb
    {
        public:
        /**
         * @brief Constructs a database administrator bound to hardware and layout providers.
         *
         * @param hwa    Hardware abstraction used for storage access.
         * @param layout Layout provider describing the common and preset regions.
         */
        Admin(Hwa&    hwa,
              Layout& layout);

        using sectionParameterType_t = lib::lessdb::SectionParameterType;
        using lib::lessdb::LessDb::read;
        using lib::lessdb::LessDb::update;

        /**
         * @brief Reads a value from the active preset region.
         *
         * @tparam T Section enum type.
         * @tparam I Parameter index type.
         *
         * @param section Preset section selector.
         * @param index   Parameter index within the section.
         *
         * @return Stored value, or zero on failure.
         */
        template<typename T, typename I>
        uint32_t read(T section, I index)
        {
            return readPreset(section, index);
        }

        /**
         * @brief Reads a value from the common region.
         *
         * @tparam I Parameter index type.
         *
         * @param section Common section selector.
         * @param index   Parameter index within the common section.
         *
         * @return Stored value, or zero on failure.
         */
        template<typename I>
        uint32_t read(Config::Section::common_t section, I index)
        {
            return readCommonBlock(static_cast<size_t>(index));
        }

        /**
         * @brief Reads a value from the active preset region into an output reference.
         *
         * @tparam T Section enum type.
         * @tparam I Parameter index type.
         *
         * @param section Preset section selector.
         * @param index   Parameter index within the section.
         * @param value   Output storage for the returned value.
         *
         * @return `true` on success, otherwise `false`.
         */
        template<typename T, typename I>
        bool read(T section, I index, uint32_t& value)
        {
            return readPreset(section, index, value);
        }

        /**
         * @brief Reads a value from the common region into an output reference.
         *
         * @tparam I Parameter index type.
         *
         * @param section Common section selector.
         * @param index   Parameter index within the common section.
         * @param value   Output storage for the returned value.
         *
         * @return Always `true`; zero is returned on read failure.
         */
        template<typename I>
        bool read(Config::Section::common_t section, I index, uint32_t& value)
        {
            value = readCommonBlock(static_cast<size_t>(index));
            return true;
        }

        /**
         * @brief Updates a value in the active preset region.
         *
         * @tparam T Section enum type.
         * @tparam I Parameter index type.
         * @tparam V Value type.
         *
         * @param section Preset section selector.
         * @param index   Parameter index within the section.
         * @param value   Value to write.
         *
         * @return `true` on success, otherwise `false`.
         */
        template<typename T, typename I, typename V>
        bool update(T section, I index, V value)
        {
            return updatePreset(section, index, value);
        }

        /**
         * @brief Updates a writable value in the common region.
         *
         * @tparam I Parameter index type.
         * @tparam V Value type.
         *
         * @param section Common section selector.
         * @param index   Parameter index within the common section.
         * @param value   Value to write.
         *
         * @return `true` on success, otherwise `false`.
         */
        template<typename I, typename V>
        bool update(Config::Section::common_t section, I index, V value)
        {
            if (static_cast<uint8_t>(index) < static_cast<uint8_t>(Config::commonSetting_t::CUSTOM_COMMON_SETTING_START))
            {
                return false;
            }

            return updateCommonBlock(static_cast<size_t>(index), value);
        }

        /**
         * @brief Initializes the database and validates the stored signature.
         *
         * @return `true` on success, otherwise `false`.
         */
        bool init();

        /**
         * @brief Initializes the database and registers event handlers.
         *
         * @param handlers Handler interface for lifecycle notifications.
         *
         * @return `true` on success, otherwise `false`.
         */
        bool init(Handlers& handlers);

        /**
         * @brief Clears storage and restores all common and preset values to defaults.
         *
         * @return `true` on success, otherwise `false`.
         */
        bool factoryReset();

        /**
         * @brief Returns the number of preset slots that fit in storage.
         *
         * @return Supported preset count.
         */
        uint8_t getSupportedPresets();

        /**
         * @brief Selects a new active preset and stores it in the common region.
         *
         * @param preset Preset index to activate.
         *
         * @return `true` on success, otherwise `false`.
         */
        bool setPreset(uint8_t preset);

        /**
         * @brief Returns the currently active preset index.
         *
         * @return Active preset index.
         */
        uint8_t getPreset();

        /**
         * @brief Returns whether initialization completed successfully.
         *
         * @return `true` if the database is initialized.
         */
        bool isInitialized();

        /**
         * @brief Registers lifecycle event handlers.
         *
         * @param handlers Handler interface to register.
         */
        void registerHandlers(Handlers& handlers);

        /**
         * @brief Enables or disables preset preservation across restart.
         *
         * @param state Preservation state to store.
         *
         * @return `true` on success, otherwise `false`.
         */
        bool setPresetPreserveState(bool state);

        /**
         * @brief Returns the stored preset preservation state.
         *
         * @return `true` when the active preset should be preserved.
         */
        bool getPresetPreserveState();

        /**
         * @brief Maps a global section enum to its containing database block.
         *
         * @param section Global section selector.
         *
         * @return Database block containing the section.
         */
        static constexpr Config::block_t BLOCK(Config::Section::global_t section)
        {
            return Config::block_t::GLOBAL;
        }

        /**
         * @brief Maps a button section enum to its containing database block.
         *
         * @param section Button section selector.
         *
         * @return Database block containing the section.
         */
        static constexpr Config::block_t BLOCK(Config::Section::button_t section)
        {
            return Config::block_t::BUTTONS;
        }

        /**
         * @brief Maps an encoder section enum to its containing database block.
         *
         * @param section Encoder section selector.
         *
         * @return Database block containing the section.
         */
        static constexpr Config::block_t BLOCK(Config::Section::encoder_t section)
        {
            return Config::block_t::ENCODERS;
        }

        /**
         * @brief Maps an analog section enum to its containing database block.
         *
         * @param section Analog section selector.
         *
         * @return Database block containing the section.
         */
        static constexpr Config::block_t BLOCK(Config::Section::analog_t section)
        {
            return Config::block_t::ANALOG;
        }

        /**
         * @brief Maps an LED section enum to its containing database block.
         *
         * @param section LED section selector.
         *
         * @return Database block containing the section.
         */
        static constexpr Config::block_t BLOCK(Config::Section::leds_t section)
        {
            return Config::block_t::LEDS;
        }

        /**
         * @brief Maps an I2C section enum to its containing database block.
         *
         * @param section I2C section selector.
         *
         * @return Database block containing the section.
         */
        static constexpr Config::block_t BLOCK(Config::Section::i2c_t section)
        {
            return Config::block_t::I2C;
        }

        /**
         * @brief Maps a touchscreen section enum to its containing database block.
         *
         * @param section Touchscreen section selector.
         *
         * @return Database block containing the section.
         */
        static constexpr Config::block_t BLOCK(Config::Section::touchscreen_t section)
        {
            return Config::block_t::TOUCHSCREEN;
        }

        private:
        enum class Context : uint8_t
        {
            COMMON,
            PRESET,
        };

        Layout&   _layout;
        Handlers* _handlers = nullptr;

        const bool INITIALIZE_DATA;

        uint32_t _presetDataStartAddress = 0;
        uint32_t _presetLayoutSize       = 0;
        uint8_t  _activePreset           = 0;
        size_t   _supportedPresets       = 0;
        uint16_t _uid                    = 0;
        bool     _initialized            = false;
        Context  _activeContext          = Context::COMMON;

        /** @brief Optional weak hook invoked after global preset data initialization. */
        void customInitGlobal();

        /** @brief Optional weak hook invoked after button preset data initialization. */
        void customInitButtons();

        /** @brief Optional weak hook invoked after encoder preset data initialization. */
        void customInitEncoders();

        /** @brief Optional weak hook invoked after analog preset data initialization. */
        void customInitAnalog();

        /** @brief Optional weak hook invoked after LED preset data initialization. */
        void customInitLEDs();

        /** @brief Optional weak hook invoked after display preset data initialization. */
        void customInitDisplay();

        /** @brief Optional weak hook invoked after touchscreen preset data initialization. */
        void customInitTouchscreen();

        /**
         * @brief Selects the common database region.
         *
         * @return `true` on success, otherwise `false`.
         */
        bool selectCommon();

        /**
         * @brief Selects the preset region for the specified preset index.
         *
         * @param preset Preset index to bind.
         *
         * @return `true` on success, otherwise `false`.
         */
        bool selectPreset(uint8_t preset);

        /**
         * @brief Returns the start address of a preset slot.
         *
         * @param preset Preset index.
         *
         * @return Byte address of the preset region.
         */
        uint32_t presetStartAddress(uint8_t preset) const;

        /**
         * @brief Checks whether the stored UID matches the current preset layout UID.
         *
         * @return `true` when the stored database signature is valid.
         */
        bool isSignatureValid();

        /**
         * @brief Stores the current preset layout UID in the common region.
         *
         * @return `true` on success, otherwise `false`.
         */
        bool setUID();

        /**
         * @brief Selects a preset without updating the stored active preset value.
         *
         * @param preset Preset index to bind.
         *
         * @return `true` on success, otherwise `false`.
         */
        bool setPresetInternal(uint8_t preset);

        /**
         * @brief Reads a value from the common settings block.
         *
         * @param index Setting index within the common block.
         *
         * @return Stored value, or zero on failure.
         */
        uint16_t readCommonBlock(size_t index);

        /**
         * @brief Reads a value from the active preset region.
         *
         * @tparam T Section enum type.
         * @tparam I Parameter index type.
         *
         * @param section Preset section selector.
         * @param index   Parameter index within the section.
         *
         * @return Stored value, or zero on failure.
         */
        template<typename T, typename I>
        uint32_t readPreset(T section, I index)
        {
            if (!selectPreset(_activePreset))
            {
                return 0;
            }

            auto blockIndex = BLOCK(section);
            auto value      = lib::lessdb::LessDb::read(static_cast<uint8_t>(blockIndex),
                                                        static_cast<uint8_t>(section),
                                                        static_cast<size_t>(index));

            return value.value_or(0);
        }

        /**
         * @brief Reads a value from the active preset region into an output reference.
         *
         * @tparam T Section enum type.
         * @tparam I Parameter index type.
         *
         * @param section Preset section selector.
         * @param index   Parameter index within the section.
         * @param value   Output storage for the returned value.
         *
         * @return `true` on success, otherwise `false`.
         */
        template<typename T, typename I>
        bool readPreset(T section, I index, uint32_t& value)
        {
            if (!selectPreset(_activePreset))
            {
                return false;
            }

            auto blockIndex = BLOCK(section);
            auto readValue  = lib::lessdb::LessDb::read(static_cast<uint8_t>(blockIndex),
                                                        static_cast<uint8_t>(section),
                                                        static_cast<size_t>(index));

            if (!readValue)
            {
                return false;
            }

            value = *readValue;
            return true;
        }

        /**
         * @brief Writes a value to the common settings block.
         *
         * @param index Setting index within the common block.
         * @param value Value to write.
         *
         * @return `true` on success, otherwise `false`.
         */
        bool updateCommonBlock(size_t index, uint16_t value);

        /**
         * @brief Updates a value in the active preset region.
         *
         * @tparam T Section enum type.
         * @tparam I Parameter index type.
         * @tparam V Value type.
         *
         * @param section Preset section selector.
         * @param index   Parameter index within the section.
         * @param value   Value to write.
         *
         * @return `true` on success, otherwise `false`.
         */
        template<typename T, typename I, typename V>
        bool updatePreset(T section, I index, V value)
        {
            if (!selectPreset(_activePreset))
            {
                return false;
            }

            auto blockIndex = BLOCK(section);
            auto newValue   = static_cast<uint32_t>(value);

            return lib::lessdb::LessDb::update(static_cast<uint8_t>(blockIndex),
                                               static_cast<uint8_t>(section),
                                               static_cast<size_t>(index),
                                               newValue);
        }

        /**
         * @brief Handles reads of common settings exposed through the system-config bridge.
         *
         * @param section System-config section selector.
         * @param index   Requested setting index.
         * @param value   Output storage for the returned value.
         *
         * @return Status code on handled requests, otherwise `std::nullopt`.
         */
        std::optional<uint8_t> sysConfigGet(sys::Config::Section::global_t section, size_t index, uint16_t& value);

        /**
         * @brief Handles writes of common settings exposed through the system-config bridge.
         *
         * @param section System-config section selector.
         * @param index   Setting index to update.
         * @param value   Value to write.
         *
         * @return Status code on handled requests, otherwise `std::nullopt`.
         */
        std::optional<uint8_t> sysConfigSet(sys::Config::Section::global_t section, size_t index, uint16_t value);
    };

    /**
     * @brief Narrow database view restricted to a selected set of preset sections.
     *
     * @tparam sections Allowed database section enum types.
     */
    template<typename... sections>
    class User
    {
        public:
        /**
         * @brief Constructs a restricted preset-data view.
         *
         * @param admin Database administrator instance to access through.
         */
        User(Admin& admin)
            : _admin(admin)
        {}

        using Config = database::Config;

        /**
         * @brief Reads a value from an allowed preset section.
         *
         * @tparam T Section enum type.
         * @tparam I Parameter index type.
         *
         * @param section Preset section selector.
         * @param index   Parameter index within the section.
         *
         * @return Stored value, or zero on failure.
         */
        template<typename T, typename I>
        typename std::enable_if<(std::is_same_v<T, sections> || ...), uint32_t>::type
        read(T section, I index)
        {
            return _admin.read(section, index);
        }

        /**
         * @brief Reads a value from an allowed preset section into an output reference.
         *
         * @tparam T Section enum type.
         * @tparam I Parameter index type.
         *
         * @param section Preset section selector.
         * @param index   Parameter index within the section.
         * @param value   Output storage for the returned value.
         *
         * @return `true` on success, otherwise `false`.
         */
        template<typename T, typename I>
        typename std::enable_if<(std::is_same_v<T, sections> || ...), bool>::type
        read(T section, I index, uint32_t& value)
        {
            return _admin.read(section, index, value);
        }

        /**
         * @brief Updates a value in an allowed preset section.
         *
         * @tparam T Section enum type.
         * @tparam I Parameter index type.
         * @tparam V Value type.
         *
         * @param section Preset section selector.
         * @param index   Parameter index within the section.
         * @param value   Value to write.
         *
         * @return `true` on success, otherwise `false`.
         */
        template<typename T, typename I, typename V>
        typename std::enable_if<(std::is_same_v<T, sections> || ...), bool>::type
        update(T section, I index, V value)
        {
            return _admin.update(section, index, value);
        }

        /**
         * @brief Returns the currently active preset index.
         *
         * @return Active preset index.
         */
        uint8_t getPreset()
        {
            return _admin.getPreset();
        }

        private:
        Admin& _admin;
    };
}    // namespace database
