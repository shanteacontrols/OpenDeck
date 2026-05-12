/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "config.h"
#include "deps.h"
#include "system/config.h"

#include <type_traits>
#include <optional>

namespace opendeck::database
{
    /**
     * @brief High-level database administrator for common and preset regions.
     *
     * This class owns the active LessDB instance, binds the common and preset
     * layouts to their storage regions, and exposes typed accessors used by
     * the rest of the application.
     */
    class Admin : public zlibs::utils::lessdb::LessDb
    {
        public:
        /**
         * @brief Constructs a database administrator bound to hardware storage.
         *
         * @param hwa Hardware abstraction used for storage access.
         */
        explicit Admin(Hwa& hwa);

        using zlibs::utils::lessdb::LessDb::read;
        using zlibs::utils::lessdb::LessDb::update;

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
            return read_preset(section, index);
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
        uint32_t read(Config::Section::Common section, I index)
        {
            return read_common_block(section, static_cast<size_t>(index));
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
            return read_preset(section, index, value);
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
        bool read(Config::Section::Common section, I index, uint32_t& value)
        {
            return read_common_block(section, static_cast<size_t>(index), value);
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
            return update_preset(section, index, value);
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
        bool update(Config::Section::Common section, I index, V value)
        {
            if ((section == Config::Section::Common::CommonSettings) &&
                (static_cast<uint8_t>(index) < static_cast<uint8_t>(Config::CommonSetting::CustomCommonSettingStart)))
            {
                return false;
            }

            return update_common_block(section, static_cast<size_t>(index), value);
        }

        /**
         * @brief Initializes the database with lifecycle handlers.
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
        bool factory_reset();

        /**
         * @brief Returns the number of preset slots that fit in storage.
         *
         * @return Supported preset count.
         */
        uint8_t supported_presets();

        /**
         * @brief Selects a new active preset and stores it in the common region.
         *
         * @param preset Preset index to activate.
         *
         * @return `true` on success, otherwise `false`.
         */
        bool set_preset(uint8_t preset);

        /**
         * @brief Returns the currently active preset index.
         *
         * @return Active preset index.
         */
        uint8_t current_preset();

        /**
         * @brief Returns whether initialization completed successfully.
         *
         * @return `true` if the database is initialized.
         */
        bool is_initialized();

        /**
         * @brief Enables or disables preset preservation across restart.
         *
         * @param state Preservation state to store.
         *
         * @return `true` on success, otherwise `false`.
         */
        bool set_preset_preserve_state(bool state);

        /**
         * @brief Returns the stored preset preservation state.
         *
         * @return `true` when the active preset should be preserved.
         */
        bool preset_preserve_state();

        /**
         * @brief Maps a global section enum to its containing database block.
         *
         * @param section Global section selector.
         *
         * @return Database block containing the section.
         */
        static constexpr Config::Block block([[maybe_unused]] Config::Section::Global section)
        {
            return Config::Block::Global;
        }

        /**
         * @brief Maps a switch section enum to its containing database block.
         *
         * @param section Switch section selector.
         *
         * @return Database block containing the section.
         */
        static constexpr Config::Block block([[maybe_unused]] Config::Section::Switch section)
        {
            return Config::Block::Switches;
        }

        /**
         * @brief Maps an encoder section enum to its containing database block.
         *
         * @param section Encoder section selector.
         *
         * @return Database block containing the section.
         */
        static constexpr Config::Block block([[maybe_unused]] Config::Section::Encoder section)
        {
            return Config::Block::Encoders;
        }

        /**
         * @brief Maps an analog section enum to its containing database block.
         *
         * @param section Analog section selector.
         *
         * @return Database block containing the section.
         */
        static constexpr Config::Block block([[maybe_unused]] Config::Section::Analog section)
        {
            return Config::Block::Analog;
        }

        /**
         * @brief Maps an OUTPUT section enum to its containing database block.
         *
         * @param section OUTPUT section selector.
         *
         * @return Database block containing the section.
         */
        static constexpr Config::Block block([[maybe_unused]] Config::Section::Outputs section)
        {
            return Config::Block::Outputs;
        }

        /**
         * @brief Maps an I2c section enum to its containing database block.
         *
         * @param section I2c section selector.
         *
         * @return Database block containing the section.
         */
        static constexpr Config::Block block([[maybe_unused]] Config::Section::I2c section)
        {
            return Config::Block::I2c;
        }

        /**
         * @brief Maps a touchscreen section enum to its containing database block.
         *
         * @param section Touchscreen section selector.
         *
         * @return Database block containing the section.
         */
        static constexpr Config::Block block([[maybe_unused]] Config::Section::Touchscreen section)
        {
            return Config::Block::Touchscreen;
        }

        private:
        /**
         * @brief Selects whether database operations target the common region or the active preset region.
         */
        enum class Context : uint8_t
        {
            Common,
            Preset,
        };

        Hwa&      _hwa;
        Handlers* _handlers                  = nullptr;
        uint32_t  _preset_data_start_address = 0;
        uint32_t  _preset_layout_size        = 0;
        uint8_t   _active_preset             = 0;
        size_t    _supported_presets         = 0;
        bool      _initialized               = false;
        Context   _active_context            = Context::Common;

        /** @brief Hook invoked after global preset data initialization. */
        void custom_init_global();

        /** @brief Hook invoked after switch preset data initialization. */
        void custom_init_switches();

        /** @brief Hook invoked after encoder preset data initialization. */
        void custom_init_encoders();

        /** @brief Hook invoked after analog preset data initialization. */
        void custom_init_analog();

        /** @brief Hook invoked after OUTPUT preset data initialization. */
        void custom_init_outputs();

        /** @brief Hook invoked after display preset data initialization. */
        void custom_init_display();

        /** @brief Hook invoked after touchscreen preset data initialization. */
        void custom_init_touchscreen();

        /**
         * @brief Applies the stored preset selection and preservation state.
         *
         * @return `true` on success, otherwise `false`.
         */
        bool load_stored_preset_state();

        /**
         * @brief Regenerates database defaults in the active storage pages.
         *
         * @return `true` on success, otherwise `false`.
         */
        bool initialize_default_data();

        /**
         * @brief Selects the common database region.
         *
         * @return `true` on success, otherwise `false`.
         */
        bool select_common();

        /**
         * @brief Selects the preset region for the specified preset index.
         *
         * @param preset Preset index to bind.
         *
         * @return `true` on success, otherwise `false`.
         */
        bool select_preset(uint8_t preset);

        /**
         * @brief Returns the start address of a preset slot.
         *
         * @param preset Preset index.
         *
         * @return Byte address of the preset region.
         */
        uint32_t preset_start_address(uint8_t preset) const;

        /**
         * @brief Checks whether the stored Uid matches the current preset layout Uid.
         *
         * @return `true` when the stored database signature is valid.
         */
        bool is_signature_valid();

        /**
         * @brief Returns the expected database signature for the current layout and target.
         *
         * @return Database signature.
         */
        uint16_t signature() const;

        /**
         * @brief Stores the current preset layout Uid in the common region.
         *
         * @return `true` on success, otherwise `false`.
         */
        bool set_signature();

        /**
         * @brief Selects a preset without updating the stored active preset value.
         *
         * @param preset Preset index to bind.
         *
         * @return `true` on success, otherwise `false`.
         */
        bool set_preset_internal(uint8_t preset);

        /**
         * @brief Reads a value from the common settings block.
         *
         * @param index Setting index within the common block.
         *
         * @return Stored value, or zero on failure.
         */
        uint16_t read_common_block(Config::Section::Common section, size_t index);

        /**
         * @brief Reads a value from a common-region section.
         *
         * @param section Common section selector.
         * @param index   Setting index within the section.
         * @param value   Output storage for the returned value.
         *
         * @return `true` on success, otherwise `false`.
         */
        bool read_common_block(Config::Section::Common section, size_t index, uint32_t& value);

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
        uint32_t read_preset(T section, I index)
        {
            if (!select_preset(_active_preset))
            {
                return 0;
            }

            auto block_index = block(section);
            auto value       = zlibs::utils::lessdb::LessDb::read(static_cast<uint8_t>(block_index),
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
        bool read_preset(T section, I index, uint32_t& value)
        {
            if (!select_preset(_active_preset))
            {
                return false;
            }

            auto block_index = block(section);
            auto read_value  = zlibs::utils::lessdb::LessDb::read(static_cast<uint8_t>(block_index),
                                                                  static_cast<uint8_t>(section),
                                                                  static_cast<size_t>(index));

            if (!read_value)
            {
                return false;
            }

            value = *read_value;
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
        bool update_common_block(Config::Section::Common section, size_t index, uint16_t value);

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
        bool update_preset(T section, I index, V value)
        {
            if (!select_preset(_active_preset))
            {
                return false;
            }

            auto block_index = block(section);
            auto new_value   = static_cast<uint32_t>(value);

            return zlibs::utils::lessdb::LessDb::update(static_cast<uint8_t>(block_index),
                                                        static_cast<uint8_t>(section),
                                                        static_cast<size_t>(index),
                                                        new_value);
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
        std::optional<uint8_t> sys_config_get(sys::Config::Section::Global section, size_t index, uint16_t& value);

        /**
         * @brief Handles writes of common settings exposed through the system-config bridge.
         *
         * @param section System-config section selector.
         * @param index   Setting index to update.
         * @param value   Value to write.
         *
         * @return Status code on handled requests, otherwise `std::nullopt`.
         */
        std::optional<uint8_t> sys_config_set(sys::Config::Section::Global section, size_t index, uint16_t value);
    };

    /**
     * @brief Narrow database view restricted to a selected set of preset sections.
     *
     * @tparam sections Allowed database section enum types.
     */
    template<typename... Sections>
    class User
    {
        public:
        /**
         * @brief Constructs a restricted preset-data view.
         *
         * @param admin Database administrator instance to access through.
         */
        explicit User(Admin& admin)
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
            requires(std::is_same_v<T, Sections> || ...)
        uint32_t
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
            requires(std::is_same_v<T, Sections> || ...)
        bool
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
            requires(std::is_same_v<T, Sections> || ...)
        bool
        update(T section, I index, V value)
        {
            return _admin.update(section, index, value);
        }

        /**
         * @brief Returns the currently active preset index.
         *
         * @return Active preset index.
         */
        uint8_t current_preset()
        {
            return _admin.current_preset();
        }

        private:
        Admin& _admin;
    };
}    // namespace opendeck::database
