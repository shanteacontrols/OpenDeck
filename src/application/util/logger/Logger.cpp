/*

Copyright 2015-2022 Igor Petrovic

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

#ifdef USE_LOGGER

#include <cstdarg>
#include <stdio.h>
#include "Logger.h"

bool Logger::write(logLevel_t level, const char* file, int line, const char* message, ...)
{
    if (level != logLevel_t::AMOUNT)
    {
        if (!_writer.write(LOG_LEVEL_STRING[static_cast<uint8_t>(level)]))
        {
            return false;
        }
    }

    snprintf(_logBuffer, sizeof(_logBuffer), "[%s:%d] ", file, line);

    if (!_writer.write(_logBuffer))
    {
        return false;
    }

    va_list args;
    va_start(args, message);
    vsnprintf(_logBuffer, sizeof(_logBuffer), message, args);
    va_end(args);

    if (!_writer.write(_logBuffer))
    {
        return false;
    }

    if (LINE_ENDING != lineEnding_t::AMOUNT)
    {
        if (!_writer.write(LINE_ENDING_STRING[static_cast<uint8_t>(LINE_ENDING)]))
        {
            return false;
        }
    }

    return true;
}

#endif