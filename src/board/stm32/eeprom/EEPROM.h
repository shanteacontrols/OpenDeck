/*

Copyright 2015-2020 Igor Petrovic

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

class EmuEEPROM
{
    public:
    enum class pageStatus_t : uint32_t
    {
        valid     = 0x00,          ///< Page containing valid data
        erased    = 0xFFFFFFFF,    ///< Page is empty
        receiving = 0xEEEEEEEE     ///< Page is marked to receive data
    };

    enum class readStatus_t : uint8_t
    {
        ok,
        noVar,
        noPage
    };

    enum class writeStatus_t : uint8_t
    {
        ok,
        pageFull,
        noPage,
        writeError
    };

    typedef struct
    {
        uint32_t startAddress;
        uint8_t  sector;
    } pageDescriptor_t;

    EmuEEPROM(pageDescriptor_t& page1, pageDescriptor_t& page2)
        : page1(page1)
        , page2(page2)
    {}

    bool          init();
    readStatus_t  read(uint16_t address, uint16_t& data);
    writeStatus_t write(uint16_t address, uint16_t data);
    bool          format();

    private:
    enum class pageOp_t : uint8_t
    {
        read,
        write
    };

    bool          findValidPage(pageOp_t operation, uint16_t& page);
    writeStatus_t writeInternal(uint16_t address, uint16_t data);
    writeStatus_t pageTransfer();
    bool          erasePageLL(uint16_t page);
    bool          write16LL(uint32_t address, uint16_t data);
    bool          write32LL(uint32_t address, uint32_t data);

    pageDescriptor_t& page1;
    pageDescriptor_t& page2;
};