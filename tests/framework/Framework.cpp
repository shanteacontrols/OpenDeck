#include "Framework.h"
#include "util/logger/Logger.h"
#include <Target.h>

#ifdef APP_USE_LOGGER
CORE_LOGGER_CREATE(APP_LOGGER, [](const char* message)
                   {
                       std::cout << message;
                       return true;
                   });
#endif

namespace
{
    void glogPrefix(std::ostream& s, const google::LogMessageInfo& l, void*)
    {
        s << "["
          << std::string(PROJECT_TARGET_NAME)
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

    LOG(INFO) << "Running tests for " << std::string(PROJECT_TARGET_NAME) << " target";

    return RUN_ALL_TESTS();
}