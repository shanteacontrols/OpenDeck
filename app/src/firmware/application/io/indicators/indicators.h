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
#include "application/io/base.h"
#include "application/messaging/messaging.h"

#include "zlibs/utils/misc/kwork_delayable.h"

namespace io::indicators
{
    class Indicators : public io::Base
    {
        public:
        static constexpr uint32_t TRAFFIC_INDICATOR_TIMEOUT_MS       = 50;
        static constexpr uint32_t FACTORY_RESET_INDICATOR_TIMEOUT_MS = 250;
        static constexpr uint32_t STARTUP_INDICATOR_TIMEOUT_MS       = 150;
        static constexpr size_t   STARTUP_INDICATOR_FLASH_COUNT      = 3;

        explicit Indicators(Hwa& hwa);

        bool   init() override;
        void   updateSingle(size_t index, bool forceRefresh = false) override;
        void   updateAll(bool forceRefresh = false) override;
        size_t maxComponentUpdateIndex() override;

        private:
        Hwa&                               _hwa;
        zlibs::utils::misc::KworkDelayable _usbInOffWork;
        zlibs::utils::misc::KworkDelayable _usbOutOffWork;
        zlibs::utils::misc::KworkDelayable _dinInOffWork;
        zlibs::utils::misc::KworkDelayable _dinOutOffWork;
        zlibs::utils::misc::KworkDelayable _bleInOffWork;
        zlibs::utils::misc::KworkDelayable _bleOutOffWork;
        zlibs::utils::misc::KworkDelayable _factoryResetBlinkWork;
        bool                               _factoryResetInProgress  = false;
        bool                               _factoryResetIndicatorIn = true;

        void   onTraffic(const messaging::MidiTrafficSignal& signal);
        void   scheduleOff(type_t type);
        type_t indicatorType(messaging::MidiTransport transport, messaging::MidiDirection direction);
        void   setInputIndicators(bool state);
        void   setOutputIndicators(bool state);
        void   indicateStartup();
        void   startFactoryResetIndication();
        void   stopFactoryResetIndication();
        void   toggleFactoryResetIndication();
    };
}    // namespace io::indicators
