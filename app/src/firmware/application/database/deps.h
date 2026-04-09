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

#include "zlibs/utils/lessdb/lessdb.h"

namespace lib::lessdb = zlibs::utils::lessdb;

namespace database
{
    /**
     * @brief Hardware abstraction interface for database storage access.
     */
    class Hwa : public lib::lessdb::Hwa
    {
        public:
        /**
         * @brief Returns whether a factory reset should populate default data.
         *
         * @return `true` when default values should be written after a clear.
         */
        virtual bool initializeDatabase() = 0;
    };

    /**
     * @brief Supplies compile-time layout descriptors for the database regions.
     *
     * The layout is injected to avoid pulling application-wide dependencies
     * into the database implementation while still allowing the final layout
     * to be defined close to the modules that contribute sections.
     */
    class Layout
    {
        public:
        virtual ~Layout() = default;

        /**
         * @brief Returns the layout shared by all preset slots.
         *
         * @return Common-region layout descriptor.
         */
        virtual std::span<const lib::lessdb::Block> commonLayout() const = 0;

        /**
         * @brief Returns the layout repeated once per preset slot.
         *
         * @return Preset-region layout descriptor.
         */
        virtual std::span<const lib::lessdb::Block> presetLayout() const = 0;

        /**
         * @brief Returns the common layout size in bytes.
         *
         * @return Common layout size.
         */
        virtual uint32_t commonLayoutSize() const = 0;

        /**
         * @brief Returns the preset layout size in bytes.
         *
         * @return Preset layout size.
         */
        virtual uint32_t presetLayoutSize() const = 0;

        /**
         * @brief Returns the UID for the common layout.
         *
         * @return Common layout UID.
         */
        virtual uint16_t commonUid() const = 0;

        /**
         * @brief Returns the UID for the preset layout.
         *
         * @return Preset layout UID.
         */
        virtual uint16_t presetUid() const = 0;
    };

    /**
     * @brief Observer interface for database lifecycle events.
     */
    class Handlers
    {
        public:
        virtual ~Handlers() = default;

        /**
         * @brief Notifies that the active preset changed.
         *
         * @param preset Newly selected preset.
         */
        virtual void presetChange(uint8_t preset) = 0;

        /**
         * @brief Notifies that a factory reset is starting.
         */
        virtual void factoryResetStart() = 0;

        /**
         * @brief Notifies that a factory reset finished.
         */
        virtual void factoryResetDone() = 0;

        /**
         * @brief Notifies that database initialization completed.
         */
        virtual void initialized() = 0;
    };
}    // namespace database
