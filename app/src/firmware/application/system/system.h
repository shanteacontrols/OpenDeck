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

#include "deps.h"
#include "config.h"
#include "layout.h"
#include "application/util/cinfo/cinfo.h"
#include "application/util/scheduler/scheduler.h"

#include "lib/sysexconf/sysexconf.h"

namespace sys
{
    class System
    {
        public:
        System(Hwa&        hwa,
               Components& components);

        bool              init();
        io::ioComponent_t run();

        private:
        enum
        {
            SCHEDULED_TASK_PRESET,
            SCHEDULED_TASK_FORCED_REFRESH,
        };

        enum class backupRestoreState_t : uint8_t
        {
            NONE,
            BACKUP,
            RESTORE
        };

        class SysExDataHandler : public lib::sysexconf::DataHandler
        {
            public:
            SysExDataHandler(System& system)
                : _system(system)
            {}

            uint8_t get(uint8_t block, uint8_t section, uint16_t index, uint16_t& value) override;
            uint8_t set(uint8_t block, uint8_t section, uint16_t index, uint16_t value) override;
            uint8_t customRequest(uint16_t request, CustomResponse& customResponse) override;
            void    sendResponse(uint8_t* array, uint16_t size) override;

            private:
            System& _system;
        };

        class DatabaseHandlers : public database::Handlers
        {
            public:
            DatabaseHandlers(System& system)
                : _system(system)
            {}

            void presetChange(uint8_t preset) override;
            void factoryResetStart() override;
            void factoryResetDone() override;
            void initialized() override;

            private:
            System& _system;
        };

        static constexpr lib::sysexconf::ManufacturerId SYS_EX_MID = {
            Config::SYSEX_MANUFACTURER_ID_0,
            Config::SYSEX_MANUFACTURER_ID_1,
            Config::SYSEX_MANUFACTURER_ID_2
        };

        Hwa&                      _hwa;
        Components&               _components;
        DatabaseHandlers          _databaseHandlers;
        SysExDataHandler          _sysExDataHandler;
        lib::sysexconf::SysExConf _sysExConf;
        util::Scheduler           _scheduler;
        util::ComponentInfo       _cInfo;
        Layout                    _layout;
        backupRestoreState_t      _backupRestoreState                                                    = backupRestoreState_t::NONE;
        io::ioComponent_t         _componentIndex                                                        = io::ioComponent_t::AMOUNT;
        size_t                    _componentUpdateIndex[static_cast<uint8_t>(io::ioComponent_t::AMOUNT)] = {};

        io::ioComponent_t      checkComponents();
        void                   checkProtocols();
        void                   backup();
        void                   forceComponentRefresh();
        std::optional<uint8_t> sysConfigGet(sys::Config::Section::global_t section, size_t index, uint16_t& value);
        std::optional<uint8_t> sysConfigSet(sys::Config::Section::global_t section, size_t index, uint16_t value);
    };
}    // namespace sys