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

#include "frame_store.h"
#include "drivers/driver_base.h"
#include "application/io/base.h"
#include "application/io/digital/buttons/buttons.h"
#include "application/io/digital/encoders/encoders.h"
#include "application/threads.h"

namespace io::digital
{
    class Digital : public io::Base
    {
        public:
        Digital(drivers::DriverBase&    driver,
                FrameStore&             frameStore,
                io::buttons::Buttons&   buttons,
                io::encoders::Encoders& encoders);

        bool   init() override;
        void   updateSingle(size_t index, bool forceRefresh = false) override;
        void   updateAll(bool forceRefresh = false) override;
        size_t maxComponentUpdateIndex() override;

        private:
        static constexpr uint32_t THREAD_SLEEP_TIME_MS = 1;

        drivers::DriverBase&    _driver;
        FrameStore&             _frameStore;
        io::buttons::Buttons&   _buttons;
        io::encoders::Encoders& _encoders;
        threads::DigitalThread  _thread;
    };
}    // namespace io::digital
