#pragma once

#define TEST_START_STRING "***Starting to run tests***"
#define TEST_START_END "***Test run finished***"

#define TESTS_START()              \
    do                             \
    {                              \
        printf(TEST_START_STRING); \
        printf("\n");              \
        UNITY_BEGIN();             \
    } while (0)

#define TESTS_END()             \
    do                          \
    {                           \
        UNITY_END();            \
        printf(TEST_START_END); \
        printf("\n");           \
    } while (0)

#define TEST_SETUP() void setUp(void)
#define TEST_TEARDOWN() void tearDown(void)

#define TEST_CASE(testName) \
    void testName()
