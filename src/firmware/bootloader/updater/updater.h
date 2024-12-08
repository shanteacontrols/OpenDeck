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

namespace updater
{
    class Updater
    {
        public:
        Updater(Hwa& hwa, const uint32_t uid);

        void feed(uint8_t data);
        void reset();

        private:
        enum class receiveStage_t : uint8_t
        {
            START,
            FW_METADATA,
            FW_CHUNK,
            END,
            AMOUNT
        };

        enum class processStatus_t : uint8_t
        {
            COMPLETE,
            INCOMPLETE,
            INVALID
        };

        using processHandler_t = processStatus_t (Updater::*)(uint8_t);

        Hwa&             _hwa;
        const uint32_t   UID;
        uint8_t          _currentStage                                                 = 0;
        size_t           _currentFwPage                                                = 0;
        uint32_t         _receivedWord                                                 = 0;
        uint32_t         _fwPageBytesReceived                                          = 0;
        uint8_t          _stageBytesReceived                                           = 0;
        uint32_t         _fwBytesReceived                                              = 0;
        uint32_t         _fwSize                                                       = 0;
        uint32_t         _receivedUID                                                  = 0;
        uint8_t          _startBytesReceived                                           = 0;
        processHandler_t _processHandler[static_cast<uint8_t>(receiveStage_t::AMOUNT)] = {
            &Updater::processStart,
            &Updater::processFwMetadata,
            &Updater::processFwChunk,
            &Updater::processEnd
        };

        processStatus_t processStart(uint8_t data);
        processStatus_t processFwMetadata(uint8_t data);
        processStatus_t processFwChunk(uint8_t data);
        processStatus_t processEnd(uint8_t data);
    };
}    // namespace updater
