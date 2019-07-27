#include <gtest/gtest.h>
#include <inttypes.h>
#include "core/src/general/RingBuffer.h"

#define BUFFER_SIZE 5

core::RingBuffer<uint8_t, BUFFER_SIZE> buffer;

TEST(RingBuffer, Init)
{
    buffer.reset();

    EXPECT_TRUE(buffer.isEmpty());
    EXPECT_FALSE(buffer.isFull());
    EXPECT_EQ(0, buffer.count());
}

TEST(RingBuffer, Insertion)
{
    buffer.reset();

    EXPECT_TRUE(buffer.insert(10));

    EXPECT_FALSE(buffer.isEmpty());
    EXPECT_FALSE(buffer.isFull());
    EXPECT_EQ(1, buffer.count());

    uint8_t value;

    EXPECT_TRUE(buffer.remove(value));
    EXPECT_EQ(10, value);

    EXPECT_EQ(0, buffer.count());
    EXPECT_TRUE(buffer.isEmpty());
    EXPECT_FALSE(buffer.isFull());

    //assign random value
    value = 147;

    //try to remove again
    EXPECT_FALSE(buffer.remove(value));

    //verify that the value hasn't changed
    EXPECT_EQ(147, value);

    //fill the entire buffer
    EXPECT_TRUE(buffer.insert(10));
    EXPECT_TRUE(buffer.insert(11));
    EXPECT_TRUE(buffer.insert(12));
    EXPECT_TRUE(buffer.insert(13));
    EXPECT_TRUE(buffer.insert(14));

    EXPECT_FALSE(buffer.isEmpty());
    EXPECT_TRUE(buffer.isFull());
    EXPECT_EQ(BUFFER_SIZE, buffer.count());

    EXPECT_TRUE(buffer.remove(value));
    EXPECT_EQ(10, value);
    EXPECT_EQ(BUFFER_SIZE - 1, buffer.count());

    EXPECT_TRUE(buffer.remove(value));
    EXPECT_EQ(11, value);
    EXPECT_EQ(BUFFER_SIZE - 2, buffer.count());

    EXPECT_TRUE(buffer.remove(value));
    EXPECT_EQ(12, value);
    EXPECT_EQ(BUFFER_SIZE - 3, buffer.count());

    EXPECT_TRUE(buffer.remove(value));
    EXPECT_EQ(13, value);
    EXPECT_EQ(BUFFER_SIZE - 4, buffer.count());

    EXPECT_TRUE(buffer.remove(value));
    EXPECT_EQ(14, value);
    EXPECT_EQ(BUFFER_SIZE - 5, buffer.count());

    EXPECT_TRUE(buffer.isEmpty());
    EXPECT_FALSE(buffer.isFull());

    //verify that overwriting isn't possible
    EXPECT_TRUE(buffer.insert(10));
    EXPECT_TRUE(buffer.insert(11));
    EXPECT_TRUE(buffer.insert(12));
    EXPECT_TRUE(buffer.insert(13));
    EXPECT_TRUE(buffer.insert(14));
    EXPECT_FALSE(buffer.insert(15));

    EXPECT_FALSE(buffer.isEmpty());
    EXPECT_TRUE(buffer.isFull());

    buffer.reset();

    EXPECT_TRUE(buffer.isEmpty());
    EXPECT_FALSE(buffer.isFull());
    EXPECT_EQ(0, buffer.count());

    EXPECT_TRUE(buffer.insert(12));
    EXPECT_FALSE(buffer.isEmpty());
    EXPECT_FALSE(buffer.isFull());
    EXPECT_EQ(1, buffer.count());
    EXPECT_TRUE(buffer.remove(value));
    EXPECT_EQ(12, value);
}