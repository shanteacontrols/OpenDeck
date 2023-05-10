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

#include <array>
#include <inttypes.h>
#include <functional>
#include "util/cinfo/CInfo.h"
#include "util/scheduler/Scheduler.h"
#include "sysex/src/SysExConf.h"
#include "CustomIDs.h"
#include "database/Database.h"
#include "protocol/ProtocolBase.h"
#include "io/IOBase.h"
#include "bootloader/FwSelector/FwSelector.h"
#include "Config.h"
#include "Layout.h"

namespace sys
{
    class Instance
    {
        public:
        using usbConnectionHandler_t = std::function<void()>;

        // Time in milliseconds after which preset change will be reported to message dispatcher.
        // It's possible that the presets could be changed rapidly in which case the system will be
        // flooded with many forced component refresh messages, normally occuring after preset change.
        // Once the preset change is notified, MIDI data for all components will be forcefully resent.
        static constexpr uint32_t PRESET_CHANGE_NOTIFY_DELAY = 500;

        // Time in milliseconds after which all internal MIDI values will be forcefully resent
        // once USB connection is detected. This delay makes sure that the data is actually sent
        // to USB software using the firmware on USB plugging it. Otherwise, the data might get
        // lost due to the data being sent out too fast (immediately on plugging it), giving the
        // MIDI software no time to react.
        static constexpr uint32_t USB_CHANGE_FORCED_REFRESH_DELAY = 1000;

        // Maximum amount of component indexes which will be checked per single run() call. All indexes aren't
        // processed in order to reduce the amount of time spent in a single run() call.
        static constexpr size_t MAX_UPDATES_PER_RUN = 16;

        class HWA
        {
            public:
            virtual ~HWA() = default;

            virtual bool init()                                                                        = 0;
            virtual void update()                                                                      = 0;
            virtual void reboot(FwSelector::fwType_t type)                                             = 0;
            virtual void registerOnUSBconnectionHandler(usbConnectionHandler_t&& usbConnectionHandler) = 0;
        };

        class Components
        {
            public:
            virtual ~Components() = default;

            virtual std::array<::io::Base*, static_cast<size_t>(::io::ioComponent_t::AMOUNT)>&          io()       = 0;
            virtual std::array<::protocol::Base*, static_cast<size_t>(::protocol::protocol_t::AMOUNT)>& protocol() = 0;
            virtual database::Admin&                                                                    database() = 0;
        };

        Instance(HWA&        hwa,
                 Components& components);

        bool              init();
        io::ioComponent_t run();

        private:
        class SysExDataHandler : public SysExConf::DataHandler
        {
            public:
            SysExDataHandler(Instance& system)
                : _system(system)
            {}

            uint8_t get(uint8_t block, uint8_t section, uint16_t index, uint16_t& value) override;
            uint8_t set(uint8_t block, uint8_t section, uint16_t index, uint16_t value) override;
            uint8_t customRequest(uint16_t request, CustomResponse& customResponse) override;
            void    sendResponse(uint8_t* array, uint16_t size) override;

            private:
            Instance& _system;
        };

        class DBhandlers : public database::Admin::Handlers
        {
            public:
            DBhandlers(Instance& system)
                : _system(system)
            {}

            void presetChange(uint8_t preset) override;
            void factoryResetStart() override;
            void factoryResetDone() override;
            void initialized() override;

            private:
            Instance& _system;
        };

        enum
        {
            SCHEDULED_TASK_PRESET,
            SCHEDULED_TASK_FORCED_REFRESH,
        };

        io::ioComponent_t checkComponents();
        void              checkProtocols();
        void              backup();
        void              forceComponentRefresh();

        HWA&                _hwa;
        Components&         _components;
        DBhandlers          _dbHandlers;
        SysExDataHandler    _sysExDataHandler;
        SysExConf           _sysExConf;
        util::Scheduler     _scheduler;
        util::ComponentInfo _cInfo;
        Layout              _layout;

        static constexpr SysExConf::manufacturerID_t SYS_EX_MID = {
            Config::SYSEX_MANUFACTURER_ID_0,
            Config::SYSEX_MANUFACTURER_ID_1,
            Config::SYSEX_MANUFACTURER_ID_2
        };

        enum class backupRestoreState_t : uint8_t
        {
            NONE,
            BACKUP,
            RESTORE
        };

        backupRestoreState_t _backupRestoreState                                                    = backupRestoreState_t::NONE;
        io::ioComponent_t    _componentIndex                                                        = io::ioComponent_t::AMOUNT;
        size_t               _componentUpdateIndex[static_cast<uint8_t>(io::ioComponent_t::AMOUNT)] = {};
    };
}    // namespace sys