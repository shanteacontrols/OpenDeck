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

#include "Config.h"
#include "core/src/general/Helpers.h"
#include "board/Board.h"
#include "updater/Updater.h"
#include "board/common/usb/descriptors/hid/Redef.h"

class BTLDRWriter : public IBTLDRWriter
{
    public:
    void erasePage(uint32_t address) override
    {
        Board::bootloader::erasePage(address);
    }

    void fillPage(uint32_t address, uint16_t data) override
    {
        Board::bootloader::fillPage(address, data);
    }

    void writePage(uint32_t address) override
    {
        Board::bootloader::writePage(address);
    }

    void apply() override
    {
        Board::bootloader::applyFw();
    }
};

namespace
{
    BTLDRWriter         btldrWriter;
    Bootloader::Updater updater(btldrWriter, false, SPM_PAGESIZE, COMMAND_STARTAPPLICATION);
}    // namespace

namespace Board
{
    namespace bootloader
    {
        void packetHandler(uint32_t data)
        {
            updater.feed(data);
        }
    }    // namespace bootloader
}    // namespace Board

int main()
{
    Board::init();

    while (1)
    {
        Board::bootloader::checkPackets();
    }
}