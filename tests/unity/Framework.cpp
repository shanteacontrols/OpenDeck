#include "Framework.h"

__attribute__((weak)) void setUp(void)
{
}

__attribute__((weak)) void tearDown(void)
{
}

std::vector<testDescriptor_t> testDescriptors;

int main()
{
    TESTS_START();

    for (size_t test = 0; test < testDescriptors.size(); test++)
    {
        std::string filename = std::string(OD_BOARD) + ":" + testDescriptors.at(test).filename;
        UnitySetTestFile(filename.c_str());
        UnityDefaultTestRun(testDescriptors.at(test).func, testDescriptors.at(test).testName.c_str(), testDescriptors.at(test).lineNr);
    }

    TESTS_END();
}