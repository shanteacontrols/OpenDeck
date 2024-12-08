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

#include "cmd_parser.h"
#include "util.h"

#include <limits.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

void CmdParser::feed(const char c)
{
    switch (c)
    {
    case 0x08:
    case 0x7F:
    {
        // backspace
        // remove a character
        if (_index > 0)
        {
            _stream.write(0x08);    // backspace
            _stream.write(0x20);    // space
            _stream.write(0x08);    // backspace

            _index--;
        }

        _commandBuffer[_index] = 0;
    }
    break;

    case '\r':
    case '\n':
    {
        _stream.print("\r\n");

        _commandBuffer[_index] = '\0';
        Util::trimSpace(_commandBuffer);
        parse(_commandBuffer);
        memset(_commandBuffer, 0, sizeof(_commandBuffer));
        _index = 0;
    }
    break;

    default:
    {
        if (isspace(c) && !_index)
        {
            // ignore leading space
        }
        else
        {
            if (0x20 <= c && c <= 0x7e &&
                _index < sizeof(_commandBuffer) - 1)    // Limit text entry to (MAX_COMMAND_LENGTH - 1) characters.
            {
                _stream.write(c);
                _commandBuffer[_index++] = c;
            }
        }
    }
    break;
    }
}

void CmdParser::parse(const char* string)
{
    size_t words         = 0;
    size_t nonWhitespace = 0;
    bool   commandValid  = false;
    size_t command       = 0;

    if (strlen(string) > 0)
    {
        words++;

        for (size_t character = 0; character < strlen(string); character++)
        {
            if (isspace(string[character]))
            {
                if (words == 1)
                {
                    for (command = 0; command < _totalCommands; command++)
                    {
                        if (!strncmp(string, _commands[command], character))
                        {
                            commandValid = true;
                            break;
                        }
                    }

                    if (!commandValid)
                        break;
                }

                if (character)
                {
                    if (!isspace(string[character - 1]))
                        words++;
                }
            }
            else
            {
                nonWhitespace++;
            }
        }

        if (!nonWhitespace)
            words = 0;
    }

    if (commandValid)
    {
        size_t pinStringStart = strlen(_commands[command]) + 1;
        int    pin            = 0;

        if (Util::str2int(pin, &string[pinStringStart], 10) == Util::str2Int_t::success)
        {
            int result = 255;

            _cmdCallback(command, pin, result);
            snprintf(_responseBuffer, sizeof(_responseBuffer), "%s ok: %d", string, result);

            _stream.println(_responseBuffer);
        }
    }
}
