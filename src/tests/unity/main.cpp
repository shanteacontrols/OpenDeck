#include "unity/src/unity.h"
#include "unity/Helpers.h"

__attribute__((weak)) void setUp(void)
{
}

__attribute__((weak)) void tearDown(void)
{
}

int main()
{
    TESTS_START();
    TESTS_EXECUTE();
    TESTS_END();
}