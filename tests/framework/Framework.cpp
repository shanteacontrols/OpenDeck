#include "Framework.h"
#include <Target.h>

#ifdef USE_LOGGER
#include "logger/Logger.h"

class LoggerWriter : public Logger::StreamWriter
{
    public:
    LoggerWriter() = default;

    bool write(const char* message) override
    {
        std::cout << message;
        return true;
    }
} _loggerWriter;
Logger logger = Logger(_loggerWriter, Logger::lineEnding_t::CRLF);
#endif

namespace
{
    void glogPrefix(std::ostream& s, const google::LogMessageInfo& l, void*)
    {
        s << "["
          << std::string(BOARD_STRING)
          << "] "
             "["
          << l.severity
          << "] "
          << "["
          << l.filename
          << ':'
          << l.line_number
          << "]";
    }
}    // namespace

int main(int argc, char* argv[])
{
    FLAGS_logtostderr = 1;
    google::InitGoogleLogging(argv[0], &glogPrefix);
    ::testing::InitGoogleTest(&argc, argv);

    LOG(INFO) << "Running tests for " << std::string(BOARD_STRING) << " board";

    return RUN_ALL_TESTS();
}