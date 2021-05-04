#pragma once

#include <vector>
#include <functional>
#include <string>
#include <cstddef>
#include <iostream>
#include "unity/src/unity.h"

typedef void (*testFunction_t)();

typedef struct
{
    testFunction_t func;
    std::string    filename;
    std::string    testName;
    int            lineNr;
} testDescriptor_t;

extern std::vector<testDescriptor_t> testDescriptors;

struct CreateTest
{
    CreateTest(testFunction_t func, std::string file, std::string name, int line)
    {
        testDescriptors.push_back({ func, file, name, line });
    }
};

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

#define TEST_CASE(testName)                                                    \
    void       testName();                                                     \
    CreateTest utStruct_##testName{ testName, __FILE__, #testName, __LINE__ }; \
    void       testName()
