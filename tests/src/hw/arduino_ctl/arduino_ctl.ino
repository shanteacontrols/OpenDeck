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

#include "board.h"
#include "cmd_parser.h"

namespace
{
    enum class command_t : uint8_t
    {
        read,
        writeLow,
        writeHigh,
        AMOUNT
    };

    const char* validCommands[static_cast<size_t>(command_t::AMOUNT)] = {
        "read",
        "write_low",
        "write_high"
    };

    void commandCallback(const size_t commandIndex, const size_t pin, int& result);

    Board     board;
    CmdParser cmdParser(Serial,
                        validCommands,
                        static_cast<size_t>(command_t::AMOUNT),
                        commandCallback);

    void commandCallback(const size_t commandIndex, const size_t pin, int& result)
    {
        switch (commandIndex)
        {
        case static_cast<uint8_t>(command_t::read):
        {
            result = board.read(pin);
        }
        break;

        case static_cast<uint8_t>(command_t::writeLow):
        {
            board.write(pin, false);
        }
        break;

        case static_cast<uint8_t>(command_t::writeHigh):
        {
            board.write(pin, true);
        }
        break;

        default:
            break;
        }
    }
}    // namespace

void setup()
{
    Serial.begin(115200);
    board.setup();
}

void loop()
{
    if (Serial.available() > 0)
    {
        cmdParser.feed(Serial.read());
    }
}
