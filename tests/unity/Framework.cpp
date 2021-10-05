#include "Framework.h"
#include <glog/logging.h>

__attribute__((weak)) void setUp(void)
{
}

__attribute__((weak)) void tearDown(void)
{
}

std::vector<testDescriptor_t> testDescriptors;

int main(int argc, char* argv[])
{
    FLAGS_logtostderr = 1;

    google::InitGoogleLogging(argv[0]);
    TESTS_START();

    for (size_t test = 0; test < testDescriptors.size(); test++)
    {
        std::string filename = std::string(BOARD_STRING) + ":" + testDescriptors.at(test).filename;
        std::cout << "Running " << filename << ":" << testDescriptors.at(test).testName << std::endl;
        UnitySetTestFile(filename.c_str());
        UnityDefaultTestRun(testDescriptors.at(test).func, testDescriptors.at(test).testName.c_str(), testDescriptors.at(test).lineNr);
    }

    TESTS_END();
}