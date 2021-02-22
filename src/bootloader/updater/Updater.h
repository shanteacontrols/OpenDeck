/*

Copyright 2015-2021 Igor Petrovic

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

#include <inttypes.h>
#include <stdlib.h>

class Updater
{
    public:
    class BTLDRWriter
    {
        public:
        virtual uint32_t pageSize(size_t index)                                  = 0;
        virtual void     erasePage(size_t index)                                 = 0;
        virtual void     fillPage(size_t index, uint32_t address, uint16_t data) = 0;
        virtual void     writePage(size_t index)                                 = 0;
        virtual void     apply()                                                 = 0;
    };

    Updater(BTLDRWriter& writer, const uint64_t startCommand, const uint32_t endCommand, const uint32_t uid)
        : _writer(writer)
        , _startCommand(startCommand)
        , _endCommand(endCommand)
        , _uid(uid)
    {}

    void feed(uint8_t data);
    void reset();

    private:
    enum class receiveStage_t : uint8_t
    {
        start,
        fwMetadata,
        fwChunk,
        end,
        AMOUNT
    };

    enum class processStatus_t : uint8_t
    {
        complete,
        incomplete,
        invalid
    };

    using processHandler_t = processStatus_t (Updater::*)(uint8_t);

    processStatus_t processStart(uint8_t data);
    processStatus_t processFwMetadata(uint8_t data);
    processStatus_t processFwChunk(uint8_t data);
    processStatus_t processEnd(uint8_t data);

    BTLDRWriter& _writer;

    uint8_t        _currentStage        = 0;
    size_t         _currentFwPage       = 0;
    uint16_t       _receivedWord        = 0;
    uint32_t       _fwPageBytesReceived = 0;
    uint8_t        _stageBytesReceived  = 0;
    uint32_t       _fwBytesReceived     = 0;
    uint32_t       _fwSize              = 0;
    uint32_t       _receivedUID         = 0;
    uint8_t        _startBytesReceived  = 0;
    const uint64_t _startCommand;
    const uint32_t _endCommand;
    const uint32_t _uid;

    processHandler_t processHandler[static_cast<uint8_t>(receiveStage_t::AMOUNT)] = {
        &Updater::processStart,
        &Updater::processFwMetadata,
        &Updater::processFwChunk,
        &Updater::processEnd
    };
};