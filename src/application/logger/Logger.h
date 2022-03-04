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

#pragma once

#include <inttypes.h>

class Logger
{
    public:
    enum class logLevel_t : uint8_t
    {
        info,
        warning,
        error,
        AMOUNT
    };

    enum class lineEnding_t : uint8_t
    {
        none,
        lf,      /// Line feed
        cr,      /// Carriage return
        lfcr,    /// Line feed + carriage return
        crlf,    /// Carriage return + line feed
        AMOUNT
    };

    class StreamWriter
    {
        public:
        virtual bool write(const char* message) = 0;
    };

    Logger(StreamWriter& writer, lineEnding_t lineEnding)
        : _writer(writer)
        , _lineEnding(lineEnding)
    {}

    bool write(logLevel_t level, const char* message, ...);

    private:
    static constexpr size_t LOG_BUFFER_SIZE = 512;
    StreamWriter&           _writer;
    const lineEnding_t      _lineEnding;
    char                    _logBuffer[LOG_BUFFER_SIZE];

    const char* _logLevelString[static_cast<uint8_t>(logLevel_t::AMOUNT)] = {
        "[INFO] ",
        "[WARNING] ",
        "[ERROR] ",
    };

    const char* _lineEndingString[static_cast<uint8_t>(lineEnding_t::AMOUNT)] = {
        "",
        "\n",
        "\r",
        "\n\r",
        "\r\n",
    };
};

#ifdef USE_LOGGER
extern Logger logger;
#define LOG_INFO(...)  logger.write(Logger::logLevel_t::info, __VA_ARGS__)
#define LOG_WARN(...)  logger.write(Logger::logLevel_t::warning, __VA_ARGS__)
#define LOG_ERROR(...) logger.write(Logger::logLevel_t::error, __VA_ARGS__)
#else
#define LOG_INFO(...)
#define LOG_WARN(...)
#define LOG_ERROR(...)
#endif