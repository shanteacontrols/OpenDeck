#include "unity/Framework.h"
#include "core/src/general/RingBuffer.h"

#define BUFFER_SIZE 5

namespace
{
    core::RingBuffer<uint8_t, BUFFER_SIZE> buffer;
}

TEST_CASE(Init)
{
    buffer.reset();

    TEST_ASSERT(buffer.isEmpty() == true);
    TEST_ASSERT(buffer.isFull() == false);
    TEST_ASSERT(0 == buffer.count());
}

TEST_CASE(Insertion)
{
    buffer.reset();

    TEST_ASSERT(buffer.insert(10) == true);

    TEST_ASSERT(buffer.isEmpty() == false);
    TEST_ASSERT(buffer.isFull() == false);
    TEST_ASSERT(1 == buffer.count());

    uint8_t value;

    TEST_ASSERT(buffer.remove(value) == true);
    TEST_ASSERT(10 == value);

    TEST_ASSERT(0 == buffer.count());
    TEST_ASSERT(buffer.isEmpty() == true);
    TEST_ASSERT(buffer.isFull() == false);

    // assign random value
    value = 147;

    // try to remove again
    TEST_ASSERT(buffer.remove(value) == false);

    // verify that the value hasn't changed
    TEST_ASSERT(147 == value);

    // fill the entire buffer
    TEST_ASSERT(true == buffer.insert(10));
    TEST_ASSERT(true == buffer.insert(11));
    TEST_ASSERT(true == buffer.insert(12));
    TEST_ASSERT(true == buffer.insert(13));
    TEST_ASSERT(true == buffer.insert(14));

    TEST_ASSERT(buffer.isEmpty() == false);
    TEST_ASSERT(buffer.isFull() == true);
    TEST_ASSERT(BUFFER_SIZE == buffer.count());

    TEST_ASSERT(buffer.remove(value) == true);
    TEST_ASSERT(10 == value);
    TEST_ASSERT(BUFFER_SIZE - 1 == buffer.count());

    TEST_ASSERT(buffer.remove(value) == true);
    TEST_ASSERT(11 == value);
    TEST_ASSERT(BUFFER_SIZE - 2 == buffer.count());

    TEST_ASSERT(buffer.remove(value) == true);
    TEST_ASSERT(12 == value);
    TEST_ASSERT(BUFFER_SIZE - 3 == buffer.count());

    TEST_ASSERT(buffer.remove(value) == true);
    TEST_ASSERT(13 == value);
    TEST_ASSERT(BUFFER_SIZE - 4 == buffer.count());

    TEST_ASSERT(buffer.remove(value) == true);
    TEST_ASSERT(14 == value);
    TEST_ASSERT(BUFFER_SIZE - 5 == buffer.count());

    TEST_ASSERT(buffer.isEmpty() == true);
    TEST_ASSERT(buffer.isFull() == false);

    // verify that overwriting isn't possible
    TEST_ASSERT(true == buffer.insert(10));
    TEST_ASSERT(true == buffer.insert(11));
    TEST_ASSERT(true == buffer.insert(12));
    TEST_ASSERT(true == buffer.insert(13));
    TEST_ASSERT(true == buffer.insert(14));
    TEST_ASSERT(buffer.insert(15) == false);

    TEST_ASSERT(buffer.isEmpty() == false);
    TEST_ASSERT(buffer.isFull() == true);

    buffer.reset();

    TEST_ASSERT(buffer.isEmpty() == true);
    TEST_ASSERT(buffer.isFull() == false);
    TEST_ASSERT(0 == buffer.count());

    TEST_ASSERT(buffer.insert(12) == true);
    TEST_ASSERT(buffer.isEmpty() == false);
    TEST_ASSERT(buffer.isFull() == false);
    TEST_ASSERT(1 == buffer.count());
    TEST_ASSERT(buffer.remove(value) == true);
    TEST_ASSERT(12 == value);
}