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

#include <Arduino.h>

class CmdParser
{
    public:
    using cmdCallback_t = void(const size_t, const size_t, int& result);

    CmdParser(Stream&        stream,
              const char**   commands,
              size_t         totalCommands,
              cmdCallback_t& cmdCallback)
        : _stream(stream)
        , _commands(commands)
        , _totalCommands(totalCommands)
        , _cmdCallback(cmdCallback)
    {}

    void feed(const char c);

    private:
    void parse(const char* string);

    Stream&                 _stream;
    const char**            _commands;
    const size_t            _totalCommands;
    cmdCallback_t&          _cmdCallback;
    uint8_t                 _index                              = 0;
    static constexpr size_t MAX_COMMAND_LENGTH                  = 50;
    char                    _commandBuffer[MAX_COMMAND_LENGTH]  = {};
    char                    _responseBuffer[MAX_COMMAND_LENGTH] = {};
};