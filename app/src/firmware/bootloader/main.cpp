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

#include "board/board.h"
#include "bootloader/updater/builder.h"
#include "sysex_parser/sysex_parser.h"
#include "fw_selector/builder.h"

#include "core/mcu.h"

class Reader
{
    public:
    Reader() = default;

    void read()
    {
        uint8_t value = 0;

        if (board::usb::readMidi(_usbMIDIpacket))
        {
            if (_sysExParser.isValidMessage(_usbMIDIpacket))
            {
                size_t dataSize = _sysExParser.dataBytes();

                if (dataSize)
                {
                    for (size_t i = 0; i < dataSize; i++)
                    {
                        if (_sysExParser.value(i, value))
                        {
                            _builderUpdater.instance().feed(value);
                        }
                    }
                }
            }
        }
    }

    private:
    updater::Builder          _builderUpdater;
    protocol::midi::UsbPacket _usbMIDIpacket;
    sysex_parser::SysExParser _sysExParser;
};

namespace
{
    fw_selector::Builder builderFwSelector;
    Reader               reader;
}    // namespace

int main()
{
    board::init();
    builderFwSelector.instance().select();

    // everything beyond this point means bootloader is active
    // otherwise jump to other firmware would have already been made

    while (1)
    {
        board::update();
        reader.read();
    }
}