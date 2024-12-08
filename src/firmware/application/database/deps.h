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

#include "lib/lessdb/lessdb.h"

namespace database
{
    class Hwa : public lib::lessdb::Hwa
    {
        public:
        virtual bool initializeDatabase() = 0;
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
            SYSTEM,
            USER,
        };

        virtual ~Layout() = default;

        virtual std::vector<lib::lessdb::Block>& layout(type_t type) = 0;
    };

    class Handlers
    {
        public:
        virtual ~Handlers() = default;

        virtual void presetChange(uint8_t preset) = 0;
        virtual void factoryResetStart()          = 0;
        virtual void factoryResetDone()           = 0;
        virtual void initialized()                = 0;
    };
}    // namespace database