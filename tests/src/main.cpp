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

#include "tests/common.h"
#include "application/util/logger/logger.h"

#include "core/mcu.h"

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