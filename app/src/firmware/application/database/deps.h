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

namespace database
{
    /**
     * @brief Hardware abstraction interface for database storage access.
     */
    class Hwa : public zlibs::utils::lessdb::Hwa
    {
        public:
        /**
         * @brief Returns whether a valid factory snapshot is provisioned.
         *
         * @return `true` when the factory snapshot can be restored.
         */
        virtual bool hasFactorySnapshot() = 0;

        /**
         * @brief Stores the current initialized database contents as a factory snapshot.
         *
         * @return `true` on success, otherwise `false`.
         */
        virtual bool storeFactorySnapshot() = 0;

        /**
         * @brief Restores the active database contents from the factory snapshot.
         *
         * @return `true` on success, otherwise `false`.
         */
        virtual bool restoreFactorySnapshot() = 0;
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
        virtual std::span<const zlibs::utils::lessdb::Block> commonLayout() const = 0;

        /**
         * @brief Returns the layout repeated once per preset slot.
         *
         * @return Preset-region layout descriptor.
         */
        virtual std::span<const zlibs::utils::lessdb::Block> presetLayout() const = 0;

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
