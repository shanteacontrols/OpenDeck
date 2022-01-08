#include "unity/Framework.h"
#include "core/src/general/RingBuffer.h"

#define BUFFER_SIZE 8

namespace
{
    core::RingBuffer<uint8_t, BUFFER_SIZE> buffer;
}

TEST_CASE(Init)
{
    buffer.reset();

    TEST_ASSERT(buffer.isEmpty() == true);
    TEST_ASSERT(buffer.isFull() == false);
    TEST_ASSERT_EQUAL_UINT32(0, buffer.count());
}

TEST_CASE(Insertion)
{
    buffer.reset();

    TEST_ASSERT(buffer.insert(10) == true);

    TEST_ASSERT(buffer.isEmpty() == false);
    TEST_ASSERT(buffer.isFull() == false);
    TEST_ASSERT_EQUAL_UINT32(1, buffer.count());

    uint8_t value;

    TEST_ASSERT(buffer.remove(value) == true);
    TEST_ASSERT_EQUAL_UINT32(10, value);

    TEST_ASSERT_EQUAL_UINT32(0, buffer.count());
    TEST_ASSERT(buffer.isEmpty() == true);
    TEST_ASSERT(buffer.isFull() == false);

    // assign random value
    value = 147;

    // try to remove again
    TEST_ASSERT(buffer.remove(value) == false);

    // verify that the value hasn't changed
    TEST_ASSERT_EQUAL_UINT32(147, value);

    // fill the entire buffer
    // buffer has room for one less element than specified
    for (size_t i = 0; i < BUFFER_SIZE - 1; i++)
        TEST_ASSERT(buffer.insert(10 + i) == true);

    TEST_ASSERT(buffer.isEmpty() == false);
    TEST_ASSERT(buffer.isFull() == true);
    TEST_ASSERT_EQUAL_UINT32(BUFFER_SIZE - 1, buffer.count());

    for (size_t i = 0; i < BUFFER_SIZE - 1; i++)
    {
        TEST_ASSERT(buffer.remove(value) == true);
        TEST_ASSERT_EQUAL_UINT32(10 + i, value);
        TEST_ASSERT_EQUAL_UINT32(BUFFER_SIZE - 2 - i, buffer.count());
    }

    TEST_ASSERT(buffer.isEmpty() == true);
    TEST_ASSERT(buffer.isFull() == false);

    // verify that overwriting isn't possible
    for (size_t i = 0; i < BUFFER_SIZE - 1; i++)
        TEST_ASSERT(buffer.insert(10 + i) == true);

    TEST_ASSERT(buffer.insert(10) == false);

    TEST_ASSERT(buffer.isEmpty() == false);
    TEST_ASSERT(buffer.isFull() == true);

    buffer.reset();

    TEST_ASSERT(buffer.isEmpty() == true);
    TEST_ASSERT(buffer.isFull() == false);
    TEST_ASSERT_EQUAL_UINT32(0, buffer.count());

    TEST_ASSERT(buffer.insert(12) == true);
    TEST_ASSERT(buffer.isEmpty() == false);
    TEST_ASSERT(buffer.isFull() == false);
    TEST_ASSERT_EQUAL_UINT32(1, buffer.count());
    TEST_ASSERT(buffer.remove(value) == true);
    TEST_ASSERT_EQUAL_UINT32(12, value);
}