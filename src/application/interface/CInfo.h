/*

Copyright 2015-2019 Igor Petrovic

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

#include "database/blocks/Blocks.h"
#include "sysex/src/SysExConf.h"

class ComponentInfo
{
    public:
    using cinfoHandler_t = bool (*)(dbBlockID_t, SysExConf::sysExParameter_t);

    ComponentInfo() = default;

    void registerHandler(cinfoHandler_t handler)
    {
        this->handler = handler;
    }

    void send(dbBlockID_t block, SysExConf::sysExParameter_t id)
    {
        if (handler != nullptr)
            handler(block, id);
    }

    private:
    ///
    /// \brief Common handler used to identify currently active component during SysEx configuration.
    /// Must be implemented externally.
    ///
    cinfoHandler_t handler = nullptr;
};