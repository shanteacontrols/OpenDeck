#pragma once

#define TESTS_START()  \
    do                 \
    {                  \
        UNITY_BEGIN(); \
    } while (0)

#define TESTS_END()         \
    do                      \
    {                       \
        return UNITY_END(); \
    } while (0)

#define TEST_SETUP()    void setUp(void)
#define TEST_TEARDOWN() void tearDown(void)

#define TEST_CASE(testName) \
    void testName()
